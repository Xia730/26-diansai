#ifndef __trailing_H
#define __trailing_H

#include <stdint.h>
#include "ti_msp_dl_config.h"

/* ── 电机开环控制 ──
 *  直接设 PWM 占空比
 *  channel 1=左轮, 2=右轮
 *  speed -999~999
 */
void Motor_Control(uint8_t channel, int16_t speed);

/* ── 速度闭环控制（编码器 PI） ──
 *  每2ms跑一次 PI，调 Motor_Control 实现速度跟踪
 *  target_l/r : 目标速度（编码器单位，与 speed_l/speed_r 同量纲）
 *  tick_ms    : 系统时钟 ms（sys_tick），内部用来计算节拍
 *  PI 系数为固定值（define 在 Motor.c）
 */
void Motor_SpeedLoop(int16_t target_l, int16_t target_r, uint32_t tick_ms);

#endif