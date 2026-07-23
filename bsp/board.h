#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>
#include <stdarg.h>
#include "ti_msp_dl_config.h"



/* 延时函数 */
void delay_us(uint32_t us);
void delay_ms(uint32_t ms);

extern volatile unsigned long tick_ms;

int mspm0_delay_ms(unsigned long num_ms);
int mspm0_get_clock_ms(unsigned long *count);
void SysTick_Init(void);



#endif