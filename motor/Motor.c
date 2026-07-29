#include "Motor.h"

// PWM 周期值 
#define PWM_PERIOD       4000          

/**
 * @brief 控制 AT8236 电机驱动
 * @param channel 1: 电机1, 2: 电机2
 * @param speed   -1000 ~ 1000 
 *                >0: 正转, <0: 反转, 0: 停止
 */
void Motor_Control(uint8_t channel, int16_t speed)
{
    uint32_t duty_cycle;
    uint32_t abs_speed;
    
    // 1. 速度限幅
    if (speed >= 1000) speed = 999;
    if (speed <= -1000) speed = -999;

    // 2. 计算绝对值和占空比
    abs_speed = (speed >= 0) ? (uint32_t)speed : (uint32_t)(-speed);
    duty_cycle = (abs_speed * PWM_PERIOD) / 1000;

    // 3. 根据通道和速度方向控制 GPIO 和 PWM
    if (channel == 1) 
    {
        if (speed == 0) 
        {
            DL_TimerG_setCaptureCompareValue(PWM_A_INST, PWM_PERIOD, GPIO_PWM_A_C0_IDX); 
            DL_TimerG_setCaptureCompareValue(PWM_A_INST, PWM_PERIOD, GPIO_PWM_A_C1_IDX); 
        }
        else if (speed > 0) 
        {
            DL_TimerG_setCaptureCompareValue(PWM_A_INST, PWM_PERIOD, GPIO_PWM_A_C0_IDX); 
            DL_TimerG_setCaptureCompareValue(PWM_A_INST, duty_cycle, GPIO_PWM_A_C1_IDX);
        }
        else 
        {
            DL_TimerG_setCaptureCompareValue(PWM_A_INST, duty_cycle, GPIO_PWM_A_C0_IDX);
            DL_TimerG_setCaptureCompareValue(PWM_A_INST, PWM_PERIOD, GPIO_PWM_A_C1_IDX); 
        }
    }
    else if (channel == 2) 
    {
        if (speed == 0) 
        {
            DL_TimerG_setCaptureCompareValue(PWM_B_INST, PWM_PERIOD, GPIO_PWM_B_C2_IDX); 
            DL_TimerG_setCaptureCompareValue(PWM_B_INST, PWM_PERIOD, GPIO_PWM_B_C3_IDX); 
        }
        else if (speed > 0) 
        {
            DL_TimerG_setCaptureCompareValue(PWM_B_INST, PWM_PERIOD, GPIO_PWM_B_C2_IDX); 
            DL_TimerG_setCaptureCompareValue(PWM_B_INST, duty_cycle, GPIO_PWM_B_C3_IDX);
        }
        else
        {
            DL_TimerG_setCaptureCompareValue(PWM_B_INST, duty_cycle, GPIO_PWM_B_C2_IDX);
            DL_TimerG_setCaptureCompareValue(PWM_B_INST, PWM_PERIOD, GPIO_PWM_B_C3_IDX); 
        }
    }
}

/* ================================================
 *  增量式 PID 速度闭环（编码器）
 *
 *  每 2ms 跑一次
 *  delta_pwm 限幅 ±100（防突变）
 *  pwm 总限幅 ±999（对接 Motor_Control）
 *
 *  ★ tuning 思路：
 *    1. KP 先调：响应速度
 *       → KP 太小：响应慢
 *       → KP 太大：抖动
 *    2. KI 补稳态误差：
 *       → 两轮不一致跑偏 → 加 KI
 *       → KI 太大：过冲
 *    3. KD 抑制超调（一般不用）
 * ================================================ */
#define KP_SPEED  0.8f  //0.7
#define KI_SPEED  0.15f 	//0.15
#define KD_SPEED  0.0f
#define SP_DT     2

void Motor_SpeedLoop(int16_t target_l, int16_t target_r, uint32_t tick_ms)
{
    static uint32_t last_tick = 0;
    static int32_t pwm_l = 0, pwm_r = 0;
    static int32_t prev_err_l = 0, prev_err_r = 0;
    static int32_t prev2_err_l = 0, prev2_err_r = 0;

    extern int32_t speed_l, speed_r;

    if (tick_ms - last_tick < SP_DT) return;
    last_tick = tick_ms;

    /* ── 左轮 ── */
    int32_t err_l = (int32_t)target_l - speed_l;
    int32_t delta_l = (int32_t)(
        KP_SPEED * (err_l - prev_err_l)
      + KI_SPEED * err_l
      + KD_SPEED * (err_l - 2 * prev_err_l + prev2_err_l)
    );
    if (delta_l > 100) delta_l = 100;
    if (delta_l < -100) delta_l = -100;
    pwm_l += delta_l;
    if (pwm_l > 999) pwm_l = 999;
    if (pwm_l < -999) pwm_l = -999;
    prev2_err_l = prev_err_l;
    prev_err_l = err_l;
    Motor_Control(1, (int16_t)pwm_l);

    /* ── 右轮 ── */
    int32_t err_r = (int32_t)target_r - speed_r;
    int32_t delta_r = (int32_t)(
        KP_SPEED * (err_r - prev_err_r)
      + KI_SPEED * err_r
      + KD_SPEED * (err_r - 2 * prev_err_r + prev2_err_r)
    );
    if (delta_r > 100) delta_r = 100;
    if (delta_r < -100) delta_r = -100;
    pwm_r += delta_r;
    if (pwm_r > 999) pwm_r = 999;
    if (pwm_r < -999) pwm_r = -999;
    prev2_err_r = prev_err_r;
    prev_err_r = err_r;
    Motor_Control(2, (int16_t)pwm_r);
}

