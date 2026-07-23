#include <stdio.h>
#include "board.h"       /* 硬件初始化, 延时, printf, UART0接收 */
#include "lcd.h"         /* LCD 图形函数 */
#include "lcd_port.h"    /* 底层 SPI (通过 board.c 实现) */






int main(void) {
		SYSCFG_DL_init();
    /* 初始化 SPI 和 GPIO 引脚 */

   // if (IMU660RB_Init() != 0) {
        // 未检测到 IMU660RB
    

    while (1) {
        // 方式 A：INT1 中断中调用 Read_IMU660RB()（推荐）
        // 方式 B：定时器按 52Hz 轮询
        // {
        //     Read_IMU660RB();
        // }

        // 获取航向角
//        float yaw   = euler.angle.yaw;     // -180° ~ 180°
//        float pitch = euler.angle.pitch;
//        float roll  = euler.angle.roll;

        // 你的应用逻辑...
        delay_ms(10);
    }
}
