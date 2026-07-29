#ifndef __BEEP_H__
#define __BEEP_H__

#include <stdint.h>

/* ================== 蜂鸣器接口 ================== */

/**
 * @brief  调
 */
void Buzzer_Beep(uint32_t t);

/**
 * @brief  定时器 10ms 中断里调用此函数，实现非阻塞控制
 */
void Buzzer_Timer10msHandler(void);

#endif