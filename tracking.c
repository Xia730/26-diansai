#include "tracking.h"
#include "Emm_V5.h"
#include "lcd.h"
#include "board.h"
#include "uart_vision.h"
#include <string.h>
#include <stdio.h>

/**********************************************************
***  二维云台目标跟踪模块 - 实现
***  功能:
***    1. 格式无关的视觉坐标解析器 (提取前两个整数)
***    2. Q16定点数PID控制器 (含抗积分饱和)
***    3. 跟踪状态机
***    4. 共享UART3总线的电机命令发送
***    5. ST7735 LCD状态显示
**********************************************************/

/* ================================================================
 * 局部辅助函数: 取绝对值
 * ================================================================ */
static inline int32_t i32_abs(int32_t x)
{
    return (x >= 0) ? x : -x;
}

/* ================================================================
 * tracking_init — 初始化跟踪系统
 * ================================================================ */
void tracking_init(tracking_system_t *sys)
{
    if (sys == NULL) return;

    memset(sys, 0, sizeof(tracking_system_t));

    sys->state        = TRACK_STATE_IDLE;
    sys->target_valid = false;

    /* --- 平移轴PID初始化 --- */
    sys->pid_pan.kp           = PID_PAN_KP_Q16;
    sys->pid_pan.ki           = PID_PAN_KI_Q16;
    sys->pid_pan.kd           = PID_PAN_KD_Q16;
    sys->pid_pan.setpoint     = CAMERA_CENTER_X;
    sys->pid_pan.integral     = 0;
    sys->pid_pan.prev_error   = 0;
    sys->pid_pan.integral_max = PID_INTEGRAL_MAX_Q16;
    sys->pid_pan.output_max   = MOTOR_MAX_PULSES;
    sys->pid_pan.motor_addr   = MOTOR_PAN_ADDR;
    sys->pid_pan.invert_dir   = 0;   /* 如方向反了, 设为1 */

    /* --- 俯仰轴PID初始化 --- */
    sys->pid_tilt.kp           = PID_TILT_KP_Q16;
    sys->pid_tilt.ki           = PID_TILT_KI_Q16;
    sys->pid_tilt.kd           = PID_TILT_KD_Q16;
    sys->pid_tilt.setpoint     = CAMERA_CENTER_Y;
    sys->pid_tilt.integral     = 0;
    sys->pid_tilt.prev_error   = 0;
    sys->pid_tilt.integral_max = PID_INTEGRAL_MAX_Q16;
    sys->pid_tilt.output_max   = MOTOR_MAX_PULSES;
    sys->pid_tilt.motor_addr   = MOTOR_TILT_ADDR;
    sys->pid_tilt.invert_dir   = 0;   /* 如方向反了, 设为1 */
}

/* ================================================================
 * tracking_parse_coords — 格式无关的数字提取解析器
 *
 * 遍历 rxCmd[] 字节缓冲区, 扫描连续的数字序列,
 * 提取前两个有效整数作为 X 和 Y 坐标.
 *
 * 支持的 MaixCAM2 输出格式:
 *   "X:123,Y:456\r\n", "(123,456)", "123,456"
 *   "x=123 y=456",  JSON: {"x":123,"y":456}
 *
 * 如果找到的数字不足2个, 则 target_valid = false.
 * ================================================================ */
bool tracking_parse_coords(tracking_system_t *sys, const uint8_t *data, uint8_t len)
{
    uint8_t  i;
    int16_t  numbers[4];        /* 最多存储4个提取到的数字 */
    uint8_t  num_count = 0;
    int32_t  accum     = 0;
    bool     in_number = false;

    if (sys == NULL || data == NULL || len == 0) {
        if (sys) sys->target_valid = false;
        return false;
    }

    /* 逐字节扫描, 提取整数序列 */
    for (i = 0; i < len; i++) {
        uint8_t ch = data[i];

        if (ch >= '0' && ch <= '9') {
            /* 数字字符: 累积 */
            accum = (accum * 10) + (int32_t)(ch - '0');
            in_number = true;

            /* 安全保护: 防止累加器溢出 */
            if (accum > 9999) accum = 9999;

        } else {
            /* 非数字字符: 当前数字结束 */
            if (in_number) {
                if (num_count < 4) {
                    numbers[num_count] = (int16_t)accum;
                    num_count++;
                }
                accum = 0;
                in_number = false;
            }
        }
    }

    /* 处理末尾的数字 (最后一个数字后面没有分隔符) */
    if (in_number && num_count < 4) {
        numbers[num_count] = (int16_t)accum;
        num_count++;
    }

    /* 至少需要2个数字才构成有效坐标 */
    if (num_count < 2) {
        sys->target_valid = false;
        return false;
    }

    /* 取前两个数字作为 X, Y */
    int16_t raw_x = numbers[0];
    int16_t raw_y = numbers[1];

    /* 范围校验: 坐标必须在相机分辨率范围内 */
    if (raw_x < 0 || raw_x >= CAMERA_WIDTH ||
        raw_y < 0 || raw_y >= CAMERA_HEIGHT) {
        sys->target_valid = false;
        return false;
    }

    sys->target_x     = raw_x;
    sys->target_y     = raw_y;
    sys->target_valid = true;

    return true;
}

/* ================================================================
 * pid_compute — Q16定点数PID核心计算
 *
 * 参数:
 *   error: 原始像素误差 (int32)
 * 返回: 带符号的脉冲增量 (int32)
 *
 * 公式:
 *   P = (Kp * error) >> 16
 *   I = (Ki * integral) >> 16,  integral 限幅于 ±integral_max
 *   D = (Kd * (error - prev_error)) >> 16
 *   output = P + I + D,  限幅于 ±output_max,  施加死区
 *
 * 抗积分饱和策略:
 *   1. 条件积分: 当P项已饱和时停止积分累积
 *   2. 积分限幅: integral 钳位于 ±integral_max
 * ================================================================ */
static int32_t pid_compute(pid_controller_t *pid, int32_t error)
{
    int32_t p_term, i_term, d_term;
    int32_t output;

    /* --- 比例项 P --- */
    p_term = (int32_t)(((int64_t)pid->kp * (int64_t)error) >> 16);

    /* ---- 积分抗饱和 ----
     * 1. 误差过零检测: 目标越过画面中心时, 旧方向的积分无效, 立即清零
     *    防止"反向积分拖拽" — 这是导致只能单向旋转的根本原因
     * 2. 条件积分: 仅当P项未饱和且误差不大时才累积积分
     * 3. 积分限幅: 钳位到 ±integral_max
     * ---- */
    if ((error > 0 && pid->prev_error < 0) ||
        (error < 0 && pid->prev_error > 0)) {
        /* 误差过零: 目标从一侧移到了另一侧, 清零积分 */
        pid->integral = 0;
    } else {
        /* 条件积分: 仅当P项未饱和时累积 */
        if (i32_abs(p_term) < pid->output_max) {
            pid->integral += error;

            /* 积分限幅 */
            if (pid->integral > pid->integral_max) {
                pid->integral = pid->integral_max;
            } else if (pid->integral < -pid->integral_max) {
                pid->integral = -pid->integral_max;
            }
        }
    }

    i_term = (int32_t)(((int64_t)pid->ki * (int64_t)pid->integral) >> 16);

    /* --- 微分项 D (基于误差变化, dt隐含在Kd中) --- */
    d_term = (int32_t)(((int64_t)pid->kd * (int64_t)(error - pid->prev_error)) >> 16);
    pid->prev_error = error;

    /* --- 求和 --- */
    output = p_term + i_term + d_term;

    /* --- 输出限幅 + 反向计算抗饱和 --- */
    if (output > pid->output_max) {
        output = pid->output_max;
        /* 输出已达正上限, 如果积分还在正向增长则回退 */
        if (pid->integral > 0 && error > 0) {
            pid->integral -= error;
        }
    } else if (output < -pid->output_max) {
        output = -pid->output_max;
        /* 输出已达负上限, 如果积分还在负向增长则回退 */
        if (pid->integral < 0 && error < 0) {
            pid->integral -= error;
        }
    }

    /* --- 死区: 小输出忽略, 防止电机抖动 --- */
    if (i32_abs(output) < MOTOR_MIN_PULSES) {
        output = 0;
    }

    return output;
}

/* ================================================================
 * tracking_pid_pan — 平移轴PID迭代
 *
 * 误差 = 画面中心X - 目标X
 * 正值 = 目标偏左 → 需向左平移
 * 负值 = 目标偏右 → 需向右平移
 * ================================================================ */
void tracking_pid_pan(tracking_system_t *sys)
{
    int32_t error;

    if (sys == NULL) return;

    error = (int32_t)CAMERA_CENTER_X - (int32_t)sys->target_x;

    /* 死区检查 */
    if (i32_abs(error) < TRACKING_DEADZONE_PX) {
        error = 0;
    }

    sys->motor_pan_pulses = pid_compute(&sys->pid_pan, error);
}

/* ================================================================
 * tracking_pid_tilt — 俯仰轴PID迭代
 *
 * 误差 = 画面中心Y - 目标Y
 * 正值 = 目标偏上 → 需向上俯仰
 * 负值 = 目标偏下 → 需向下俯仰
 * ================================================================ */
void tracking_pid_tilt(tracking_system_t *sys)
{
    int32_t error;

    if (sys == NULL) return;

    error = (int32_t)CAMERA_CENTER_Y - (int32_t)sys->target_y;

    /* 死区检查 */
    if (i32_abs(error) < TRACKING_DEADZONE_PX) {
        error = 0;
    }

    sys->motor_tilt_pulses = pid_compute(&sys->pid_tilt, error);
}

/* ================================================================
 * tracking_prepare_motor_commands — 电机命令准备
 *
 * 将带符号的PID输出转换为:
 *   方向: 正值→CW(0), 负值→CCW(1)
 *   脉冲: 取绝对值
 * 支持通过 invert_dir 实现机械安装方向反转
 * ================================================================ */
void tracking_prepare_motor_commands(tracking_system_t *sys)
{
    if (sys == NULL) return;

    /* --- 平移轴 --- */
    if (sys->motor_pan_pulses >= 0) {
        sys->motor_pan_dir = 0;   /* CW */
    } else {
        sys->motor_pan_dir = 1;   /* CCW */
        sys->motor_pan_pulses = -sys->motor_pan_pulses;
    }
    /* 方向反转 (适配不同机械安装方向) */
    sys->motor_pan_dir ^= sys->pid_pan.invert_dir;

    /* --- 俯仰轴 --- */
    if (sys->motor_tilt_pulses >= 0) {
        sys->motor_tilt_dir = 0;
    } else {
        sys->motor_tilt_dir = 1;
        sys->motor_tilt_pulses = -sys->motor_tilt_pulses;
    }
    sys->motor_tilt_dir ^= sys->pid_tilt.invert_dir;
}

/* ================================================================
 * tracking_send_motor_commands — 发送电机控制命令
 *
 * 脉冲数超过 MOTOR_MIN_PULSES 死区时才发送.
 * 使用增量定位模式 (raF=false), 立即执行 (snF=false).
 * 命令之间加入延时, 防止共享UART3总线冲突.
 * ================================================================ */
void tracking_send_motor_commands(tracking_system_t *sys)
{
    if (sys == NULL) return;

    /* --- 平移电机 --- */
    if (sys->motor_pan_pulses >= MOTOR_MIN_PULSES) {
        Emm_V5_Pos_Control(
            MOTOR_PAN_ADDR,
            sys->motor_pan_dir,
            MOTOR_VEL_RPM,
            MOTOR_ACC,
            (uint32_t)sys->motor_pan_pulses,
            false,   /* raF=false: 相对当前位置增量运动 */
            false    /* snF=false: 立即执行, 不等待同步触发 */
        );

        /* 短暂延时, 等待UART发送完成, 避免与下一条命令冲突 */
        delay_ms(MOTOR_CMD_DELAY_MS);
    }

    /* --- 俯仰电机 --- */
    if (sys->motor_tilt_pulses >= MOTOR_MIN_PULSES) {
        Emm_V5_Pos_Control(
            MOTOR_TILT_ADDR,
            sys->motor_tilt_dir,
            MOTOR_VEL_RPM,
            MOTOR_ACC,
            (uint32_t)sys->motor_tilt_pulses,
            false,
            false
        );

        delay_ms(MOTOR_CMD_DELAY_MS);
    }
}

/* ================================================================
 * tracking_state_name — 状态→字符串
 * ================================================================ */
const char* tracking_state_name(track_state_t state)
{
    switch (state) {
        case TRACK_STATE_IDLE:      return "IDLE";
        case TRACK_STATE_SEARCHING: return "SEARCH";
        case TRACK_STATE_LOCKED:    return "LOCKED";
        case TRACK_STATE_LOST:      return "LOST";
        default:                    return "????";
    }
}

/* ================================================================
 * tracking_update_state — 跟踪状态机更新
 *
 * 状态转移:
 *   IDLE      -> SEARCHING  (收到首个有效帧, 开始搜索)
 *   SEARCHING -> LOCKED     (连续有效帧, 建立跟踪锁定)
 *   LOCKED    -> LOST       (超过 OBJ_LOST_TIMEOUT_MS 无有效帧)
 *   LOST      -> SEARCHING  (有效帧重新出现, 恢复搜索)
 *
 * 进入LOST状态时自动清零PID积分项, 防止重新锁定时的积分累积.
 * ================================================================ */
void tracking_update_state(tracking_system_t *sys, bool frame_valid, uint32_t tick_ms)
{
    if (sys == NULL) return;

    /* 记录有效帧的时间戳 */
    if (frame_valid) {
        sys->last_frame_tick = tick_ms;
        sys->frame_count++;
    }

    switch (sys->state) {

        case TRACK_STATE_IDLE:
            /* 首次收到有效数据 → 进入搜索状态 */
            if (frame_valid) {
                sys->state = TRACK_STATE_SEARCHING;
            }
            break;

        case TRACK_STATE_SEARCHING:
            if (frame_valid) {
                /* 连续收到有效帧 → 锁定目标 */
                sys->state = TRACK_STATE_LOCKED;
            }
            break;

        case TRACK_STATE_LOCKED:
            /* 超时无有效帧 → 目标丢失 */
            if (!frame_valid &&
                (tick_ms - sys->last_frame_tick) > OBJ_LOST_TIMEOUT_MS) {
                sys->state = TRACK_STATE_LOST;
                /* 清零积分项, 防止重新锁定时积分饱和 */
                sys->pid_pan.integral   = 0;
                sys->pid_tilt.integral  = 0;
                sys->pid_pan.prev_error = 0;
                sys->pid_tilt.prev_error = 0;
            }
            break;

        case TRACK_STATE_LOST:
            /* 目标重新出现 → 进入搜索 */
            if (frame_valid) {
                sys->state = TRACK_STATE_SEARCHING;
            }
            break;

        default:
            sys->state = TRACK_STATE_IDLE;
            break;
    }
}

/* ================================================================
 * tracking_lcd_update — LCD状态刷新
 *
 * 显示布局 (128x160 ST7735, 8x8点阵字体):
 *   y=0:   "== Gimbal Tracker =="  黄色, 静态标题
 *   y=20:  "S:LOCKED        "     白色, 动态状态
 *   y=40:  "Tgt: 160, 120   "     绿色(有效) / 红色(丢失)
 *   y=56:  "Err:+040, -025   "    青色, 像素误差
 *   y=72:  "Pan:+0067 D0     "    青色, 平移脉冲+方向
 *   y=88:  "Tilt:-0042 D1    "    青色, 俯仰脉冲+方向
 *   y=108: "Frm:12345        "    橙色, 帧计数
 * ================================================================ */
void tracking_lcd_update(tracking_system_t *sys)
{
    int32_t error_x, error_y;

    if (sys == NULL) return;

    error_x = (int32_t)CAMERA_CENTER_X - (int32_t)sys->target_x;
    error_y = (int32_t)CAMERA_CENTER_Y - (int32_t)sys->target_y;

    /* 标题行 */
    lcd_printf(0, 5, YELLOW, BLACK, "== Gimbal Tracker ==");

    /* 状态行 */
    lcd_printf(0, 20, WHITE, BLACK, "S:%-8s          ",
               tracking_state_name(sys->state));

    /* 目标坐标行 */
    if (sys->target_valid) {
        lcd_printf(0, 35, GREEN, BLACK,
                   "Tgt:%+04d,%+04d    ", sys->target_x, sys->target_y);
    } else {
        lcd_printf(0, 35, RED, BLACK, "Tgt: ---, ---    ");
    }

    /* 误差行 */
    lcd_printf(0, 51, CYAN, BLACK,
               "Err:%+04ld,%+04ld    ", error_x, error_y);

    /* 平移电机行 */
    lcd_printf(0, 65, CYAN, BLACK,
               "Pan:%+04ld D%u       ",
               sys->motor_pan_pulses, sys->motor_pan_dir);

    /* 俯仰电机行 */
    lcd_printf(0, 83, CYAN, BLACK,
               "Tilt:%+04ld D%u      ",
               sys->motor_tilt_pulses, sys->motor_tilt_dir);

    /* 帧计数行 */
    lcd_printf(0, 93, ORANGE, BLACK,
               "Frm:%lu            ", sys->frame_count);
}

/* ================================================================
 * tracking_get_tick_ms
 *
 * 此处仅为声明占位, 实际实现在 empty.c (主应用程序) 中.
 * ================================================================ */
