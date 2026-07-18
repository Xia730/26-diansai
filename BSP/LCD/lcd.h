#ifndef __LCD_H
#define __LCD_H

#include "lcd_init.h"
#include "lcdfont.h"

/*========================================================
    LCD 屏幕参数
========================================================*/
#define LCD_WIDTH           128     /* 液晶宽度 (像素)      */
#define LCD_HEIGHT          160     /* 液晶高度 (像素)      */

/*========================================================
    RGB565 常用颜色定义
========================================================*/
#define WHITE               0xFFFF
#define BLACK               0x0000
#define BLUE                0x001F
#define BRED                0xF81F
#define GRED                0xFFE0
#define GBLUE               0x07FF
#define RED                 0xF800
#define MAGENTA             0xF81F
#define GREEN               0x07E0
#define CYAN                0x7FFF
#define YELLOW              0xFFE0
#define BROWN               0xBC40
#define BRRED               0xFC07
#define GRAY                0x8430
#define DARKBLUE            0x01CF
#define LIGHTBLUE           0x7D7C
#define GRAYBLUE            0x5458
#define LIGHTGREEN          0x841F
#define LGRAY               0xC618
#define LGRAYBLUE           0xA651
#define LBBLUE              0x2B12

/*========================================================
    宏定义 - 颜色合成
========================================================*/
/* 从8位R,G,B合成16位RGB565颜色 */
#define LCD_RGB565(r, g, b)     ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

/*========================================================
    辅助函数
========================================================*/
uint32_t LCD_Pow(uint32_t m, uint32_t n);       /* m的n次方 */

/*========================================================
    函数声明 - 底层操作
========================================================*/
void LCD_WR_REG(uint8_t reg);                       /* 写寄存器(命令) */
void LCD_WR_DATA8(uint8_t dat);                     /* 写8位数据       */
void LCD_WR_DATA16(uint16_t dat);                   /* 写16位数据      */
void LCD_WriteRAM_Prepare(void);                    /* 准备写GRAM      */

/*========================================================
    函数声明 - 区域设置
========================================================*/
void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

/*========================================================
    函数声明 - 绘制函数
========================================================*/
void LCD_Clear(uint16_t color);                         /* 清屏              */
void LCD_Fill(uint16_t x1, uint16_t y1,                 /* 填充矩形          */
              uint16_t x2, uint16_t y2, uint16_t color);
void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color); /* 画点         */
void LCD_DrawLine(uint16_t x1, uint16_t y1,              /* 画线              */
                  uint16_t x2, uint16_t y2, uint16_t color);
void LCD_DrawRectangle(uint16_t x1, uint16_t y1,         /* 画空心矩形        */
                       uint16_t x2, uint16_t y2, uint16_t color);
void LCD_DrawCircle(uint16_t x, uint16_t y,              /* 画空心圆          */
                    uint16_t r, uint16_t color);
void LCD_DrawCircleFill(uint16_t x, uint16_t y,           /* 画实心圆          */
                        uint16_t r, uint16_t color);

/*========================================================
    函数声明 - 字符/字符串显示
========================================================*/
void LCD_ShowChar(uint16_t x, uint16_t y,                /* 显示一个字符      */
                  uint8_t chr, uint8_t size, uint16_t mode);
void LCD_ShowString(uint16_t x, uint16_t y,              /* 显示字符串        */
                    const uint8_t *str, uint8_t size, uint16_t mode);
void LCD_ShowNum(uint16_t x, uint16_t y,                 /* 显示数字          */
                 uint32_t num, uint8_t len, uint8_t size);
void LCD_ShowFloat(uint16_t x, uint16_t y,               /* 显示浮点数        */
                   float num, uint8_t len, uint8_t size, uint8_t decPlaces);
void LCD_ShowChinese(uint16_t x, uint16_t y,             /* 显示中文 (12x12)  */
                     uint8_t index, uint16_t mode);

/*========================================================
    函数声明 - 图片显示
========================================================*/
void LCD_ShowPicture(uint16_t x, uint16_t y,            /* 显示图片          */
                     uint16_t w, uint16_t h, const uint8_t *p);

/*========================================================
    函数声明 - 背光控制
========================================================*/
void LCD_BLK_Set(uint8_t state);                        /* 背光开关          */

/*========================================================
    函数声明 - printf格式化输出
========================================================*/
void LCD_printf(uint16_t x, uint16_t y,                 /* 格式化字符串显示  */
                uint8_t size, const char *format, ...);

#endif /* __LCD_H */
