#include "ti_msp_dl_config.h"
#include "board.h"
#include "lcd.h"



int main(void)
{
    SYSCFG_DL_init();
    LCD_GPIO_Init();      // 初始化GPIO + 硬件SPI
      LCD_Init();           // ST7735 初始化序列

      LCD_Clear(BLACK);     // 清黑屏

      LCD_ShowString(0, 0, (uint8_t *)"Hello ST7735!", 16, 1);
      LCD_DrawLine(0, 20, 127, 20, RED);
      LCD_DrawCircle(64, 80, 30, BLUE);
    while (1)
    {
		}
    
	
}
