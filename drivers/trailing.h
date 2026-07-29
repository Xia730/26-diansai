#ifndef _trailing_H_
#define _trailing_H_

#include "ti_msp_dl_config.h"

#define START_BYTE   '#'//帧头
#define END_BYTE     '!'//帧尾
#define FRAME_LEN    12//数据长度

extern uint8_t ascii_buf[FRAME_LEN + 1];//缓冲数组
extern uint8_t bin_array[12];//最终数组

extern volatile uint8_t bin_ready ;
extern volatile uint8_t buf_index ;
extern volatile uint8_t receiving ;



void UART3_SendStates(volatile uint8_t bin_array[12]);    // 发送函数
void UART_Process(uint8_t rx);                            //中断接收函数
void UART0_INST_IRQHandler(void);                        // UART0 接收灰度数据中断

/* ── 循迹转向 PID 计算（灰度传感器 → 差速值） ──
 *
 *  增量式 PI，每 5ms 计算一次，不直接操作电机
 *  调用方拿到 diff 后与 base_speed 合成 target_l/r，再调 Motor_SpeedLoop
 *
 *  算法：
 *    1. 计算黑线重心位置（12 路传感器加权平均）
 *    2. 重心偏差 = 重心 - 5.5（中心线）
 *    3. 增量式 PID → 差速值
 *
 *  sensor[12] : 灰度传感器，0=白 1=黑（在线上）
 *  tick_ms    : 系统时钟 ms
 *
 *  返回：
 *    diff_speed — 正值=线偏左→向右修正（右轮加速/左轮减速）
 *                 0  = 线居中，直走
 *
 *  无黑线时（sum=0）：跳过计算，返回上次 diff
 *  PID 系数 #define 在 trailing.c（TRAIL_KP / TRAIL_KI / TRAIL_KD）
 */
int16_t Trail_Steering_Compute(const uint8_t sensor[12], uint32_t tick_ms);

/* ── 全黑检测（启停线） ──
 *  12 路传感器全部为黑时返回 1
 */
uint8_t Trail_AllBlack(const uint8_t sensor[12]);

/* ── 停止线检测（带消抖） ──
 *  检测到后需先离开（<3 路黑）才能再次触发
 */
uint8_t Trail_DetectStopLine(const uint8_t sensor[12]);

#endif