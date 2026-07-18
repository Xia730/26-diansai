#include "board.h"


/*  
		延时函数
    延时i毫秒
*/
void delay_ms(uint32_t i)
{
    uint32_t clycle;
    clycle = i * (CPUCLK_FREQ/1000);
    delay_cycles(clycle);
}
