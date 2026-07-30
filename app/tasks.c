#include "tasks.h"
#include "trailing.h"
#include "motor/Motor.h"
#include "lcd.h"

extern volatile uint32_t sys_tick;
extern int32_t speed_l, speed_r;
extern volatile int32_t enc_count_l, enc_count_r;

/* ================================================
 *  全局变量定义
 *  tasks.h 里声明为 extern，这里提供实际内存
 *
 *  g_active_task : 当前正在运行的任务（null=没有任务）
 *  g_task_run    : 1=运行, 0=暂停（MENU_PAUSED时清0）
 *                  主循环靠这个判断要不要调 run()
 *  current_idx   : 当前选中的是任务几（用于显示）
 * ================================================ */
const Task *g_active_task = 0;
volatile uint8_t g_task_run = 0;
static uint8_t current_idx = 0;

/* ================================================
 *  ★★ 以下是你写的任务逻辑 ★★
 *
 *  每个任务三个函数：
 *    init() — 进入任务时调用一次（重置变量用）
 *    run()  — 主循环每次循环都调（控制逻辑写这里）
 *    stop() — 退出任务时调用一次（清理用）
 *
 *  主循环已把控制和界面分离：
 *    控制：每次 while 循环都调 run()，无延迟
 *    界面：每10ms刷一次 LCD/按键，不影响控制
 *
 *  run() 的参数：
 *    s[0~11] : 12个灰度传感器，0=白 1=黑
 *    t        : 系统当前毫秒数（可用于超时/计时判断）
 *
 *  你可以在 run() 内部自己控制节奏：
 *    方式A：等传感器同步 — if (!bin_ready) return;
 *    方式B：定时运行     — if (t - last < interval) return;
 *    方式C：全力运行     — 不等待，跑满 CPU
 *
 *  可调参数（菜单 Settings 页实时修改）：
 *    param_base_speed  — 基础速度
 *    param_turn_speed  — 转弯速度
 *    param_pid_p       — PID比例（×100，如150=1.50）
 *    param_pid_i       — PID积分
 *    param_pid_d       — PID微分
 *    param_threshold   — 阈值
 * ================================================ */

/* ──── 任务1：循迹 + 一圈计时停车 ────
 *
 *  K3 启动 → 记录编码器起点 → 开始计时循迹
 *  0~16.5s：正常速度，不检测停止线
 *  16.5s后：线性降速
 *    编码器增量达标 + 传感器检测到停止线 → 停车显示时间
 *
 *  状态机：WAIT(启动) → LAP(循迹) → DONE(停车)
 */
#define T1_WAIT  0
#define T1_LAP   1
#define T1_DONE  2

static uint32_t t1_start_tick = 0;
static uint32_t t1_final_time = 0;
static uint8_t  t1_phase = 0;
static int32_t  t1_enc_l_start = 0;
static int32_t  t1_enc_r_start = 0;

static void task1_init(void)
{
    t1_start_tick = 0;
    t1_phase = T1_WAIT;
    t1_enc_l_start = enc_count_l;
    t1_enc_r_start = enc_count_r;
}

static void task1_run(uint8_t *s, uint32_t t)
{
    uint32_t elapsed = t - t1_start_tick;
    int32_t enc_avg = ((enc_count_l - t1_enc_l_start) +
                        (enc_count_r - t1_enc_r_start)) / 2;

    // 状态机
    switch (t1_phase) {
    case T1_WAIT:
        t1_start_tick = t;
        t1_phase = T1_LAP;
        break;
    case T1_LAP:
        if (Trail_DetectStopLine(s) || enc_avg >= 10050)
            t1_phase = T1_DONE;
        break;
    case T1_DONE:
        t1_final_time = elapsed;
        TASK_Stop();
        return;
    }

    // 缓启动：1秒内从0线性升到目标速度
    // 编码器 ≥9500 开始降速，到 10050 时降到 200
    int16_t eff_speed;
    if (elapsed < 1000) {
        eff_speed = (int16_t)((int32_t)param_base_speed * elapsed / 1000);
    } else if (enc_avg >= 9500) {
        int32_t ramp = enc_avg - 9500;
        int32_t reduction = ((int32_t)param_base_speed - 200) * ramp / 550;
        eff_speed = (int16_t)param_base_speed - (int16_t)reduction;
        if (eff_speed < 200) eff_speed = 200;
    } else {
        eff_speed = (int16_t)param_base_speed;
    }

    int16_t diff = Trail_Steering_Compute(s, t);
    int16_t target_l = eff_speed - diff;
    int16_t target_r = eff_speed + diff;

    if (target_l < -1000) target_l = -1000;
    if (target_l >  1000) target_l =  1000;
    if (target_r < -1000) target_r = -1000;
    if (target_r >  1000) target_r =  1000;

    Motor_SpeedLoop(target_l, target_r, t);
}

static void task1_stop(void) {}

static void task1_draw(void)
{
    lcd_printf(0, 2, BLUE, WHITE, "Trail 1 Lap");
    if (t1_phase == T1_WAIT) {
        lcd_printf(0, 30, BLACK, WHITE, "Waiting...");
    } else if (t1_phase == T1_DONE) {
        lcd_printf(0, 30, RED, WHITE, "%lu.%02lu s  DONE",
                   t1_final_time / 1000, (t1_final_time % 1000) / 10);
    } else {
        uint32_t elapse = sys_tick - t1_start_tick;
        lcd_printf(0, 30, BLACK, WHITE, "%lu.%02lu s",
                   elapse / 1000, (elapse % 1000) / 10);
        // 编码器进度 + 穿越次数
        int32_t enc_avg = ((enc_count_l - t1_enc_l_start) +
                            (enc_count_r - t1_enc_r_start)) / 2;
        lcd_printf(0, 56, BLACK, WHITE, "Enc:%d",
                   enc_avg);
    }
    lcd_printf(0, 84, BLACK, WHITE,
               "Spd:%d L:%d R:%d", param_base_speed, speed_l, speed_r);
    lcd_printf(0, 112, GRAY, WHITE, "K3 Pause K4 Stop");
}

/* ──── 任务2：循迹 + 编码器5000停止 ──── */
static uint32_t t2_start_tick = 0;
static int32_t  t2_enc_start = 0;

static void task2_init(void)
{
    t2_start_tick = 0;
    t2_enc_start = (enc_count_l + enc_count_r) / 2;
}

static void task2_run(uint8_t *s, uint32_t t)
{
    uint32_t elapsed = t - t2_start_tick;
    int32_t  enc_avg = ((enc_count_l + enc_count_r) / 2) - t2_enc_start;

    if (enc_avg >= 5000) {
        TASK_Stop();
        return;
    }

    // 缓启动 + 编码器降速
    int16_t eff_speed;
    if (elapsed < 1000) {
        eff_speed = (int16_t)((int32_t)param_base_speed * elapsed / 1000);
    } else if (enc_avg >= 4000) {
        int32_t ramp = enc_avg - 4000;
        int32_t reduction = (int32_t)param_base_speed * ramp / 1000;
        eff_speed = (int16_t)param_base_speed - (int16_t)reduction;
        if (eff_speed < 0) eff_speed = 0;
    } else {
        eff_speed = (int16_t)param_base_speed;
    }

    int16_t diff = Trail_Steering_Compute(s, t);
    int16_t target_l = eff_speed - diff;
    int16_t target_r = eff_speed + diff;
    if (target_l < -1000) target_l = -1000;
    if (target_l >  1000) target_l =  1000;
    if (target_r < -1000) target_r = -1000;
    if (target_r >  1000) target_r =  1000;
    Motor_SpeedLoop(target_l, target_r, t);
}

static void task2_stop(void) {}

static void task2_draw(void)
{
    int32_t enc_avg = ((enc_count_l + enc_count_r) / 2) - t2_enc_start;
    lcd_printf(0, 2, BLUE, WHITE, "A->B");
    lcd_printf(0, 30, BLACK, WHITE, "Enc:%d/5000", enc_avg);
    lcd_printf(0, 58, BLACK, WHITE,
               "Spd:%d L:%d R:%d", param_base_speed, speed_l, speed_r);
    lcd_printf(0, 86, GRAY, WHITE, "K3 Pause K4 Stop");
}

/* ──── 任务3：循迹 + 编码器11000停止 ──── */
static uint32_t t3_start_tick = 0;
static int32_t  t3_enc_start = 0;

static void task3_init(void)
{
    t3_start_tick = 0;
    t3_enc_start = (enc_count_l + enc_count_r) / 2;
}

static void task3_run(uint8_t *s, uint32_t t)
{
    uint32_t elapsed = t - t3_start_tick;
    int32_t  enc_avg = ((enc_count_l + enc_count_r) / 2) - t3_enc_start;

    if (enc_avg >= 11000) {
        TASK_Stop();
        return;
    }

    // 缓启动 + 编码器降速到0
    int16_t eff_speed;
    if (elapsed < 1000) {
        eff_speed = (int16_t)((int32_t)param_base_speed * elapsed / 1000);
    } else if (enc_avg >= 9500) {
        int32_t ramp = enc_avg - 9500;
        int32_t reduction = (int32_t)param_base_speed * ramp / 1500;
        eff_speed = (int16_t)param_base_speed - (int16_t)reduction;
        if (eff_speed < 0) eff_speed = 0;
    } else {
        eff_speed = (int16_t)param_base_speed;
    }

    int16_t diff = Trail_Steering_Compute(s, t);
    int16_t target_l = eff_speed - diff;
    int16_t target_r = eff_speed + diff;
    if (target_l < -1000) target_l = -1000;
    if (target_l >  1000) target_l =  1000;
    if (target_r < -1000) target_r = -1000;
    if (target_r >  1000) target_r =  1000;
    Motor_SpeedLoop(target_l, target_r, t);
}

static void task3_stop(void) {}

static void task3_draw(void)
{
    int32_t enc_avg = ((enc_count_l + enc_count_r) / 2) - t3_enc_start;
    lcd_printf(0, 2, BLUE, WHITE, "A->A");
    lcd_printf(0, 30, BLACK, WHITE, "Enc:%d/11000", enc_avg);
    lcd_printf(0, 58, BLACK, WHITE,
               "Spd:%d L:%d R:%d", param_base_speed, speed_l, speed_r);
    lcd_printf(0, 86, GRAY, WHITE, "K3 Pause K4 Stop");
}

/* ================================================
 *  任务注册表
 * ================================================ */
const Task tasks[TASK_COUNT] = {
    { task1_init, task1_run, task1_stop, task1_draw },
    { task2_init, task2_run, task2_stop, task2_draw },
    { task3_init, task3_run, task3_stop, task3_draw },
};

/* ================================================
 *  任务管理函数
 *
 *  TASK_Select(idx) : 选中一个任务
 *    1. 停掉当前任务（如果有）
 *    2. 记录当前索引
 *    3. 调用新任务的 init()
 *    4. 设置 g_task_run = 1（主循环开始调 run()）
 *
 *  TASK_Stop() : 停止当前任务
 *    1. 调 stop()
 *    2. g_active_task = null（主循环不再调 run()）
 *    3. 电机强制停止
 * ================================================ */
void TASK_Select(uint8_t idx)
{
    if (idx >= TASK_COUNT) return;
    if (g_active_task) g_active_task->stop();
    current_idx = idx;
    g_active_task = &tasks[idx];
    g_active_task->init();
    g_task_run = 1;
}

void TASK_Stop(void)
{
    if (g_active_task) g_active_task->stop();
    g_active_task = 0;
    g_task_run = 0;
    Motor_Control(1, 0);
    Motor_Control(2, 0);
}

uint8_t TASK_GetCurrentIdx(void)
{
    return current_idx;
}
