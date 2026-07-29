/**
 * @file    lcd.c
 * @brief   ST7735 LCD 应用层 — 绘图 + 文字 + 格式化输出 (DMA)
 */
#include "lcd.h"
#include "lcd_front.h"

/* DMA 开关: 0=轮询 1=DMA */
#define LCD_FILL_DMA   1   /* SINGLE 模式 DMA */
#define LCD_CHAR_DMA   1

/* ========== 共享 DMA 像素 buffer ========== */
static u16 g_pixel_buf[LCD_DMA_BUF_SIZE / 2];  /* 256 像素 */

/* ========== 辅助宏 ========== */
#define SWAP16(c)  ((u16)(((c) >> 8) | ((c) << 8)))

/* ========== LCD_Fill — DMA 批量填充 ========== */

void LCD_Fill(u16 x1, u16 y1, u16 x2, u16 y2, u16 color)
{
    u32 n = (u32)(x2 - x1) * (u32)(y2 - y1);
    u32 i, chunk, rem;
    u16 cs;

    if (n == 0) return;

    LCD_Address_Set(x1, y1, x2 - 1, y2 - 1);

    cs = SWAP16(color);
    for (i = 0; i < 256; i++) g_pixel_buf[i] = cs;

    rem = n;
    while (rem) {
        chunk = (rem > 256) ? 256 : rem;
#if LCD_FILL_DMA
        LCD_WR_DATA_DMA(g_pixel_buf, chunk);
#else
        for (u32 pp = 0; pp < chunk; pp++) LCD_WR_DATA16(SWAP16(g_pixel_buf[pp]));
#endif
        rem -= chunk;
    }
}

/* ========== LCD_DrawPoint ========== */

void LCD_DrawPoint(u16 x, u16 y, u16 color)
{
    LCD_Address_Set(x, y, x, y);
    LCD_WR_DATA16(color);
}

/* ========== LCD_DrawLine — Bresenham ========== */

void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2, u16 color)
{
    u16 t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, uRow, uCol;

    delta_x = (int)x2 - (int)x1;
    delta_y = (int)y2 - (int)y1;
    uRow = x1;
    uCol = y1;

    incx = (delta_x > 0) ? 1 : ((delta_x == 0) ? 0 : -1);
    incy = (delta_y > 0) ? 1 : ((delta_y == 0) ? 0 : -1);
    if (delta_x < 0) delta_x = -delta_x;
    if (delta_y < 0) delta_y = -delta_y;

    distance = (delta_x > delta_y) ? delta_x : delta_y;

    for (t = 0; t <= (u16)distance; t++) {
        LCD_DrawPoint(uRow, uCol, color);
        xerr += delta_x;
        yerr += delta_y;
        if (xerr > distance) { xerr -= distance; uRow += incx; }
        if (yerr > distance) { yerr -= distance; uCol += incy; }
    }
}

/* ========== LCD_DrawRectangle ========== */

void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2, u16 color)
{
    LCD_DrawLine(x1, y1, x2, y1, color);
    LCD_DrawLine(x1, y1, x1, y2, color);
    LCD_DrawLine(x1, y2, x2, y2, color);
    LCD_DrawLine(x2, y1, x2, y2, color);
}

/* ========== Draw_Circle ========== */

void Draw_Circle(u16 x0, u16 y0, u8 r, u16 color)
{
    int a, b;
    a = 0; b = r;
    while (a <= b) {
        LCD_DrawPoint(x0 - b, y0 - a, color);
        LCD_DrawPoint(x0 + b, y0 - a, color);
        LCD_DrawPoint(x0 - a, y0 + b, color);
        LCD_DrawPoint(x0 - a, y0 - b, color);
        LCD_DrawPoint(x0 + b, y0 + a, color);
        LCD_DrawPoint(x0 + a, y0 - b, color);
        LCD_DrawPoint(x0 + a, y0 + b, color);
        LCD_DrawPoint(x0 - b, y0 + a, color);
        a++;
        if ((a * a + b * b) > (r * r)) b--;
    }
}

/* ========== LCD_ShowChar — buffer → DMA ========== */

void LCD_ShowChar(u16 x, u16 y, u8 num, u16 fc, u16 bc, u8 sizey, u8 mode)
{
    u8  sizex = sizey / 2;
    u16 TypefaceNum = (u16)(sizex / 8 + ((sizex % 8) ? 1 : 0)) * sizey;
    u16 i;
    u8  t, temp, m = 0;
    u16 x0 = x;
    u32 pixel_idx;

    if (num < ' ') return;
    num -= ' ';

    LCD_Address_Set(x, y, x + sizex - 1, y + sizey - 1);

    if (!mode) {
        /* —— DMA 路径 —— */
        u16 fcs = SWAP16(fc);
        u16 bcs = SWAP16(bc);
        pixel_idx = 0;

        for (i = 0; i < TypefaceNum; i++) {
            temp = ascii_1206[num][i];
            for (t = 0; t < 8; t++) {
                g_pixel_buf[pixel_idx++] = (temp & (0x01 << t)) ? fcs : bcs;
                m++;
                if (m % sizex == 0) { m = 0; break; }
            }
        }
#if LCD_CHAR_DMA
        LCD_WR_DATA_DMA(g_pixel_buf, pixel_idx);
#else
        for (u32 pp = 0; pp < pixel_idx; pp++) LCD_WR_DATA16(SWAP16(g_pixel_buf[pp]));
#endif
    } else {
        /* —— 叠加模式 (逐点) —— */
        for (i = 0; i < TypefaceNum; i++) {
            temp = ascii_1206[num][i];
            for (t = 0; t < 8; t++) {
                if (temp & (0x01 << t)) LCD_DrawPoint(x, y, fc);
                x++;
                if ((x - x0) == sizex) { x = x0; y++; break; }
            }
        }
    }
}

/* ========== LCD_ShowString ========== */

void LCD_ShowString(u16 x, u16 y, const u8 *p, u16 fc, u16 bc, u8 sizey, u8 mode)
{
    u8 sizex = sizey / 2;
    u16 x0 = x;
    while (*p) {
        if (*p == '\n') { x = x0; y += sizey; p++; continue; }
        if (*p == '\r') { p++; continue; }
        LCD_ShowChar(x, y, *p, fc, bc, sizey, mode);
        x += sizex;
        p++;
    }
}

/* ========== mypow ========== */

u32 mypow(u8 m, u8 n)
{
    u32 result = 1;
    while (n--) result *= m;
    return result;
}

/* ========== LCD_ShowIntNum ========== */

void LCD_ShowIntNum(u16 x, u16 y, u16 num, u8 len, u16 fc, u16 bc, u8 sizey)
{
    u8  t, temp, sizex = sizey / 2;
    u8  enshow = 0;

    for (t = 0; t < len; t++) {
        temp = (u8)((num / mypow(10, len - t - 1)) % 10);
        if (enshow == 0 && t < (len - 1)) {
            if (temp == 0) {
                LCD_ShowChar(x + t * sizex, y, ' ', fc, bc, sizey, 0);
                continue;
            }
            enshow = 1;
        }
        LCD_ShowChar(x + t * sizex, y, temp + '0', fc, bc, sizey, 0);
    }
}

/* ========== LCD_ShowFloatNum1 — 1位小数 ========== */

void LCD_ShowFloatNum1(u16 x, u16 y, float num, u8 len, u16 fc, u16 bc, u8 sizey)
{
    u8  t, temp, sizex = sizey / 2;
    u16 num1 = (u16)(num * 10.0f + 0.5f);  /* 1位小数 → 整数 */
    u8  orig_len = len;

    for (t = 0; t < orig_len; t++) {
        temp = (u8)((num1 / mypow(10, orig_len - t - 1)) % 10);
        if (t == (orig_len - 1)) {
            /* 小数点位置 */
            LCD_ShowChar(x + (orig_len - 1) * sizex, y, '.', fc, bc, sizey, 0);
            t++;
            orig_len += 1;
        }
        LCD_ShowChar(x + t * sizex, y, temp + '0', fc, bc, sizey, 0);
    }
}

/* ========== lcd_printf — 格式化输出 (6x12 字体, 自动清残影) ========== */

void lcd_printf(u16 x, u16 y, u16 fg, u16 bg,
                const char *format, ...)
{
    char buf[64];

    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);

    /* 先写字 — 避免白底闪现 */
    LCD_ShowString(x, y, (const u8 *)buf, fg, bg, 12, 0);

    /* 再填本行尾部 — 清除旧字符残影 (自动填到屏幕最右边) */
    size_t len = strlen(buf);
    u16 tail_x = x + (u16)len * 6;
    if (tail_x < LCD_W) {
        LCD_Fill(tail_x, y, LCD_W, y + 12, bg);
    }
}