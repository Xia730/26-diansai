#include "tasks.h"
#include "trailing.h"
#include "motor/Motor.h"
#include "lcd.h"

extern volatile uint32_t sys_tick;
extern int32_t speed_l, speed_r;

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
 *  K3 启动 → 等 12 路全黑 → 开始计时循迹
 *  → 检测到 A 点横线（带消抖） → 停车显示时间
 *
 *  状态机：WAIT(等全黑启动) → LAP(循迹) → DONE(停车)
 */
#define T1_WAIT  0
#define T1_LAP   1
#define T1_DONE  2

static uint32_t t1_start_tick = 0;
static uint32_t t1_final_time = 0;
static uint8_t  t1_phase = 0;

static void task1_init(void)
{
    t1_start_tick = 0;
    t1_phase = T1_WAIT;
}

static void task1_run(uint8_t *s, uint32_t t)
{
    // 比赛模式：启停 + 计时
    switch (t1_phase) {
    case T1_WAIT:
        if (Trail_AllBlack(s)) {
            t1_start_tick = t;
            t1_phase = T1_LAP;
        }
        Motor_Control(1, 0);
        Motor_Control(2, 0);
        return;
    case T1_LAP:
        if (Trail_DetectStopLine(s)) t1_phase = T1_DONE;
        break;
    case T1_DONE:
        t1_final_time = t - t1_start_tick;
        TASK_Stop();
        return;
    }

    int16_t diff = Trail_Steering_Compute(s, t);
    int16_t target_l = (int16_t)param_base_speed - diff;
    int16_t target_r = (int16_t)param_base_speed + diff;

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
        lcd_printf(0, 24, BLACK, WHITE, "Waiting...");
    } else if (t1_phase == T1_DONE) {
        lcd_printf(0, 24, RED, WHITE, "%lu.%02lu s  DONE",
                   t1_final_time / 1000, (t1_final_time % 1000) / 10);
    } else {
        uint32_t elapse = sys_tick - t1_start_tick;
        lcd_printf(0, 24, BLACK, WHITE, "%lu.%02lu s",
                   elapse / 1000, (elapse % 1000) / 10);
    }
    lcd_printf(0, 44, BLACK, WHITE,
               "Spd:%d L:%d R:%d", param_base_speed, speed_l, speed_r);
    lcd_printf(0, 66, GRAY, WHITE, "K3 Pause K4 Stop");
}

/* ──── 任务2：速度环直行测试 ──── */
static void task2_init(void) {}
static void task2_run(uint8_t *s, uint32_t t)
{
    (void)s;
    Motor_SpeedLoop((int16_t)param_base_speed,
                    (int16_t)param_base_speed, t);
}
static void task2_stop(void) {}

/* ──── 任务3：一圈计时 ────
 *
 *  K3 启动 → 开始计时 → 循迹一圈 → 检测到 A 点停止线 → 自动停车
 *  显示已用时间
 *
 *  状态机：
 *    PHASE_START    刚启动，等传感器离开起始横线
 *    PHASE_LAP      循迹中，等待回到 A 点
 *    PHASE_DONE     完成，停车显示时间
 */
#define PHASE_START  0
#define PHASE_LAP    1
#define PHASE_DONE   2

static uint32_t lap_start_tick = 0;
static uint8_t  lap_phase = 0;

static void task3_init(void)
{
    lap_start_tick = 0;
    lap_phase = PHASE_START;
}

static void task3_run(uint8_t *s, uint32_t t)
{
    /* 首次运行记录起始时间 */
    if (lap_start_tick == 0) {
        lap_start_tick = t;
    }

    uint32_t elapsed = t - lap_start_tick;

    /* ── 状态机 ── */
    switch (lap_phase) {

    case PHASE_START:
        /* 先离开起始横线再开始等下一圈 */
        {
            uint8_t cnt = 0;
            for (uint8_t i = 0; i < 12; i++) if (s[i]) cnt++;
            if (cnt < 3) lap_phase = PHASE_LAP;
        }
        /* 循迹不能停 */
        break;

    case PHASE_LAP:
        if (Trail_DetectStopLine(s)) {
            lap_phase = PHASE_DONE;
        }
        break;

    case PHASE_DONE:
        TASK_Stop();
        return;
    }

    /* ── 正常循迹 ── */
    int16_t diff = Trail_Steering_Compute(s, t);
    int16_t target_l = (int16_t)param_base_speed - diff;
    int16_t target_r = (int16_t)param_base_speed + diff;

    if (target_l < -1000) target_l = -1000;
    if (target_l >  1000) target_l =  1000;
    if (target_r < -1000) target_r = -1000;
    if (target_r >  1000) target_r =  1000;

    Motor_SpeedLoop(target_l, target_r, t);
}

static void task3_stop(void) {}

static void task3_draw(void)
{
    lcd_printf(0, 2, BLUE, WHITE, "Lap Timer");
    lcd_printf(0, 24, BLACK, WHITE, "%lu.%02lu s",
               (lap_start_tick ? (sys_tick - lap_start_tick) / 1000 : 0),
               (lap_start_tick ? ((sys_tick - lap_start_tick) % 1000) / 10 : 0));
    lcd_printf(0, 44, BLACK, WHITE,
               "Spd:%d L:%d R:%d", param_base_speed, speed_l, speed_r);
    lcd_printf(0, 66, GRAY, WHITE, "K3 Pause K4 Stop");
}

/* ──── 任务4~5：空模板 ──── */
static void task4_init(void) {}
static void task4_run(uint8_t *s, uint32_t t) { (void)s; (void)t; }
static void task4_stop(void) {}

static void task5_init(void) {}
static void task5_run(uint8_t *s, uint32_t t) { (void)s; (void)t; }
static void task5_stop(void) {}

/* ================================================
 *  任务注册表
 *
 *  把上面5个任务的 init/run/stop 函数注册到这里
 *  菜单选"任务1"时，TASK_Select(0) 就会调用
 *  tasks[0].init → tasks[0].run → ... → tasks[0].stop
 *
 *  TASK_COUNT = 5 定义在 params.h
 * ================================================ */
const Task tasks[TASK_COUNT] = {
    { task1_init, task1_run, task1_stop, task1_draw },
    { task2_init, task2_run, task2_stop, 0 },
    { task3_init, task3_run, task3_stop, task3_draw },
    { task4_init, task4_run, task4_stop, 0 },
    { task5_init, task5_run, task5_stop, 0 },
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
