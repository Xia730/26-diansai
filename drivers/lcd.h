/**
 * @file    lcd.h
 * @brief   ST7735 LCD 应用层 — 绘图 + 文字 + 格式化输出
 */
#ifndef __LCD_H
#define __LCD_H

#include "lcd_port.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========== 类型别名 ========== */
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;

/* ========== 颜色 (RGB565) ========== */
#define WHITE       0xFFFF
#define BLACK       0x0000
#define BLUE        0x001F
#define RED         0xF800
#define MAGENTA     0xF81F
#define GREEN       0x07E0
#define CYAN        0x7FFF
#define YELLOW      0xFFE0
#define GRAY        0x8430
#define DARKBLUE    0x01CF
#define LIGHTBLUE   0x7D7C
#define LGRAY       0xC618
#define BROWN       0xBC40
#define BRED        0xF81F
#define BRRED       0xFC07
#define GRAYBLUE    0x5458
#define LIGHTGREEN  0x841F
#define LGRAYBLUE   0xA651
#define LBBLUE      0x2B12

/* ========== 绘图 API ========== */
void LCD_Fill(u16 x1, u16 y1, u16 x2, u16 y2, u16 color);
void LCD_DrawPoint(u16 x, u16 y, u16 color);
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2, u16 color);
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2, u16 color);
void Draw_Circle(u16 x0, u16 y0, u8 r, u16 color);

/* ========== 文字 API ========== */
void LCD_ShowChar(u16 x, u16 y, u8 num, u16 fc, u16 bc, u8 sizey, u8 mode);
void LCD_ShowString(u16 x, u16 y, const u8 *p, u16 fc, u16 bc, u8 sizey, u8 mode);
u32 mypow(u8 m, u8 n);
void LCD_ShowIntNum(u16 x, u16 y, u16 num, u8 len, u16 fc, u16 bc, u8 sizey);
void LCD_ShowFloatNum1(u16 x, u16 y, float num, u8 len, u16 fc, u16 bc, u8 sizey);

/* ========== 格式化输出 ========== */
void lcd_printf(u16 x, u16 y, u16 fg, u16 bg,
                const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* __LCD_H */
