#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "board.h"




/*------------------------------------------------
作用    : 延时指定纳秒数
参数    : __us: 纳秒数
返回值  : 无
------------------------------------------------*/
void delay_us(uint32_t __us)
{
    delay_cycles((CPUCLK_FREQ / 1000000) * __us);
}

/*------------------------------------------------
作用    : 延时指定毫秒数
参数    : __ms: 毫秒数
返回值  : 无
------------------------------------------------*/
void delay_ms(uint32_t __ms)
{
    delay_cycles((CPUCLK_FREQ / 1000) * __ms);
}


volatile unsigned long tick_ms;
volatile uint32_t start_time;

int mspm0_delay_ms(unsigned long num_ms)
{
    start_time = tick_ms;
    while (tick_ms - start_time < num_ms);
    return 0;
}

int mspm0_get_clock_ms(unsigned long *count)
{
    if (!count)
        return 1;
    count[0] = tick_ms;
    return 0;
}

void SysTick_Init(void)
{
    DL_SYSTICK_config(CPUCLK_FREQ/1000);
    NVIC_SetPriority(SysTick_IRQn, 0);
}




