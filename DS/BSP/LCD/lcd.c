#include "lcd.h"
#include "lcd_init.h"
#include "ti/driverlib/dl_spi.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

/*========================================================
    全局变量
========================================================*/
/* 显存缓冲区 - 存储当前屏幕显示内容 */
uint16_t LCD_GRAM[LCD_WIDTH][LCD_HEIGHT];

/*========================================================
    内部函数声明
========================================================*/
static void LCD_WriteCmd(uint8_t cmd);
static void LCD_WriteData(uint8_t dat);
static void LCD_WriteData16(uint16_t dat);
static void LCD_WriteBurstData(const uint8_t *buf, uint32_t len);

/*========================================================
    名称: LCD_Pow
    作用: 计算 m 的 n 次方 (用于数字/浮点数显示)
========================================================*/
uint32_t LCD_Pow(uint32_t m, uint32_t n)
{
    uint32_t result = 1;
    while (n--)
    {
        result *= m;
    }
    return result;
}

/*========================================================
    名称: LCD_WriteCmd
    作用: 通过硬件SPI写命令到ST7735
    参数: cmd - 8位命令
========================================================*/
static void LCD_WriteCmd(uint8_t cmd)
{
    DL_GPIO_clearPins(LCD_DC_PORT, LCD_DC_PIN);     /* DC=0: 命令 */
    DL_GPIO_clearPins(LCD_CS_PORT, LCD_CS_PIN);
    DL_SPI_transmitDataBlocking8(LCD_SPI_INST, cmd);
    DL_GPIO_setPins(LCD_CS_PORT, LCD_CS_PIN);
}

/*========================================================
    名称: LCD_WriteData
    作用: 通过硬件SPI写单字节数据到ST7735
    参数: dat - 8位数据
========================================================*/
static void LCD_WriteData(uint8_t dat)
{
    DL_GPIO_setPins(LCD_DC_PORT, LCD_DC_PIN);       /* DC=1: 数据 */
    DL_GPIO_clearPins(LCD_CS_PORT, LCD_CS_PIN);
    DL_SPI_transmitDataBlocking8(LCD_SPI_INST, dat);
    DL_GPIO_setPins(LCD_CS_PORT, LCD_CS_PIN);
}

/*========================================================
    名称: LCD_WriteData16
    作用: 通过硬件SPI写16位数据到ST7735 (RGB565颜色值)
    参数: dat - 16位数据, 先发高字节
========================================================*/
static void LCD_WriteData16(uint16_t dat)
{
    DL_GPIO_setPins(LCD_DC_PORT, LCD_DC_PIN);       /* DC=1: 数据 */
    DL_GPIO_clearPins(LCD_CS_PORT, LCD_CS_PIN);
    DL_SPI_transmitDataBlocking8(LCD_SPI_INST, (uint8_t)(dat >> 8));
    DL_SPI_transmitDataBlocking8(LCD_SPI_INST, (uint8_t)(dat));
    DL_GPIO_setPins(LCD_CS_PORT, LCD_CS_PIN);
}

/*========================================================
    名称: LCD_WriteBurstData
    作用: 连续写入多个字节数据 (用于批量像素填充)
    参数: buf - 数据缓冲区指针
    参数: len - 写入字节数
    说明: CS和DC在此期间保持不变, 提高写入效率
========================================================*/
static void LCD_WriteBurstData(const uint8_t *buf, uint32_t len)
{
    uint32_t i;

    DL_GPIO_setPins(LCD_DC_PORT, LCD_DC_PIN);       /* DC=1: 数据 */
    DL_GPIO_clearPins(LCD_CS_PORT, LCD_CS_PIN);

    for (i = 0; i < len; i++)
    {
        DL_SPI_transmitDataBlocking8(LCD_SPI_INST, buf[i]);
    }

    DL_GPIO_setPins(LCD_CS_PORT, LCD_CS_PIN);
}

/*========================================================
    名称: LCD_WR_REG
    作用: 对外接口 - 写寄存器(命令)
========================================================*/
void LCD_WR_REG(uint8_t reg)
{
    LCD_WriteCmd(reg);
}

/*========================================================
    名称: LCD_WR_DATA8
    作用: 对外接口 - 写8位数据
========================================================*/
void LCD_WR_DATA8(uint8_t dat)
{
    LCD_WriteData(dat);
}

/*========================================================
    名称: LCD_WR_DATA16
    作用: 对外接口 - 写16位数据
========================================================*/
void LCD_WR_DATA16(uint16_t dat)
{
    LCD_WriteData16(dat);
}

/*========================================================
    名称: LCD_WriteRAM_Prepare
    作用: 准备写入GRAM (发送RAMWR命令, 后续可直接写颜色数据)
========================================================*/
void LCD_WriteRAM_Prepare(void)
{
    LCD_WriteCmd(ST7735_RAMWR);
}

/*========================================================
    名称: LCD_Address_Set
    作用: 设置液晶显示区域 (列地址和行地址)
    参数: x1, y1 - 区域左上角坐标
    参数: x2, y2 - 区域右下角坐标
========================================================*/
void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    LCD_WriteCmd(ST7735_CASET);     /* 列地址设置 */
    LCD_WriteData16(x1);
    LCD_WriteData16(x2);

    LCD_WriteCmd(ST7735_RASET);     /* 行地址设置 */
    LCD_WriteData16(y1);
    LCD_WriteData16(y2);

    LCD_WriteCmd(ST7735_RAMWR);     /* 内存写入 */
}

/*========================================================
    名称: LCD_Clear
    作用: 全屏清屏为指定颜色
    参数: color - 16位RGB565颜色值
========================================================*/
void LCD_Clear(uint16_t color)
{
    uint16_t i;
    uint32_t total;

    /* 设置全屏区域 */
    LCD_Address_Set(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);

    total = (uint32_t)LCD_WIDTH * (uint32_t)LCD_HEIGHT;

    /* 硬件SPI连续写入 - 高效批量填充 */
    DL_GPIO_setPins(LCD_DC_PORT, LCD_DC_PIN);       /* DC=1: 数据 */
    DL_GPIO_clearPins(LCD_CS_PORT, LCD_CS_PIN);

    for (i = 0; i < total; i++)
    {
        DL_SPI_transmitDataBlocking8(LCD_SPI_INST, (uint8_t)(color >> 8));
        DL_SPI_transmitDataBlocking8(LCD_SPI_INST, (uint8_t)(color));
    }

    DL_GPIO_setPins(LCD_CS_PORT, LCD_CS_PIN);

    /* 刷新显存 */
    for (i = 0; i < LCD_WIDTH; i++)
    {
        uint16_t j;
        for (j = 0; j < LCD_HEIGHT; j++)
        {
            LCD_GRAM[i][j] = color;
        }
    }
}

/*========================================================
    名称: LCD_Fill
    作用: 填充指定矩形区域
    参数: x1, y1 - 区域左上角坐标
    参数: x2, y2 - 区域右下角坐标
    参数: color - 16位RGB565颜色值
========================================================*/
void LCD_Fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    uint16_t x, y;
    uint32_t count;

    count = (uint32_t)(x2 - x1 + 1) * (uint32_t)(y2 - y1 + 1);

    LCD_Address_Set(x1, y1, x2, y2);

    DL_GPIO_setPins(LCD_DC_PORT, LCD_DC_PIN);
    DL_GPIO_clearPins(LCD_CS_PORT, LCD_CS_PIN);

    while (count--)
    {
        DL_SPI_transmitDataBlocking8(LCD_SPI_INST, (uint8_t)(color >> 8));
        DL_SPI_transmitDataBlocking8(LCD_SPI_INST, (uint8_t)(color));
    }

    DL_GPIO_setPins(LCD_CS_PORT, LCD_CS_PIN);

    /* 刷新显存 */
    for (x = x1; x <= x2 && x < LCD_WIDTH; x++)
    {
        for (y = y1; y <= y2 && y < LCD_HEIGHT; y++)
        {
            LCD_GRAM[x][y] = color;
        }
    }
}

/*========================================================
    名称: LCD_DrawPoint
    作用: 在指定位置画一个像素点
    参数: x - x坐标 (0 ~ LCD_WIDTH-1)
    参数: y - y坐标 (0 ~ LCD_HEIGHT-1)
    参数: color - 16位RGB565颜色值
========================================================*/
void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color)
{
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT)
        return;

    LCD_Address_Set(x, y, x, y);
    LCD_WriteData16(color);

    LCD_GRAM[x][y] = color;
}

/*========================================================
    名称: LCD_DrawLine
    作用: 画直线 (Bresenham算法)
    参数: x1, y1 - 起点坐标
    参数: x2, y2 - 终点坐标
    参数: color - 16位RGB565颜色值
========================================================*/
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    int16_t dx, dy;
    int16_t sx, sy;
    int16_t err, e2;
    int16_t x, y;

    dx = (int16_t)abs((int16_t)x2 - (int16_t)x1);
    dy = (int16_t)abs((int16_t)y2 - (int16_t)y1);

    if (x1 < x2) sx = 1; else sx = -1;
    if (y1 < y2) sy = 1; else sy = -1;

    err = (dx > dy ? dx : -dy) / 2;
    x = x1;
    y = y1;

    while (1)
    {
        LCD_DrawPoint(x, y, color);

        if (x == x2 && y == y2) break;

        e2 = err;
        if (e2 > -dx) { err -= dy; x += sx; }
        if (e2 <  dy) { err += dx; y += sy; }
    }
}

/*========================================================
    名称: LCD_DrawRectangle
    作用: 画空心矩形
========================================================*/
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    LCD_DrawLine(x1, y1, x2, y1, color);
    LCD_DrawLine(x2, y1, x2, y2, color);
    LCD_DrawLine(x2, y2, x1, y2, color);
    LCD_DrawLine(x1, y2, x1, y1, color);
}

/*========================================================
    名称: LCD_DrawCircle
    作用: 画空心圆 (Bresenham中点画圆算法)
    参数: x, y - 圆心坐标
    参数: r - 半径
========================================================*/
void LCD_DrawCircle(uint16_t x, uint16_t y, uint16_t r, uint16_t color)
{
    int16_t a, b, di;

    if (r == 0) return;

    a = 0;
    b = r;
    di = 3 - (r << 1);

    while (a <= b)
    {
        LCD_DrawPoint(x + a, y + b, color);
        LCD_DrawPoint(x + b, y + a, color);
        LCD_DrawPoint(x - a, y + b, color);
        LCD_DrawPoint(x - b, y + a, color);
        LCD_DrawPoint(x + a, y - b, color);
        LCD_DrawPoint(x + b, y - a, color);
        LCD_DrawPoint(x - a, y - b, color);
        LCD_DrawPoint(x - b, y - a, color);

        a++;

        if (di < 0)
        {
            di += (a << 2) + 6;
        }
        else
        {
            di += 10 + ((a - b) << 2);
            b--;
        }
    }
}

/*========================================================
    名称: LCD_DrawCircleFill
    作用: 画实心圆
    参数: x, y - 圆心坐标
    参数: r - 半径
========================================================*/
void LCD_DrawCircleFill(uint16_t x, uint16_t y, uint16_t r, uint16_t color)
{
    int16_t a, b, di;
    int16_t i;

    if (r == 0) return;

    a = 0;
    b = r;
    di = 3 - (r << 1);

    while (a <= b)
    {
        for (i = x - a; i <= x + a; i++) { LCD_DrawPoint(i, y - b, color); }
        for (i = x - a; i <= x + a; i++) { LCD_DrawPoint(i, y + b, color); }
        for (i = x - b; i <= x + b; i++) { LCD_DrawPoint(i, y - a, color); }
        for (i = x - b; i <= x + b; i++) { LCD_DrawPoint(i, y + a, color); }

        a++;

        if (di < 0)
        {
            di += (a << 2) + 6;
        }
        else
        {
            di += 10 + ((a - b) << 2);
            b--;
        }
    }
}

/*========================================================
    名称: LCD_ShowChar
    作用: 在指定位置显示一个ASCII字符
    参数: x, y  - 左上角坐标
    参数: chr  - 要显示的字符 (ASCII码)
    参数: size - 字体大小: 8(6x8), 12(6x12), 16(8x16), 24(12x24)
    参数: mode - 0:反色显示, 1:正常显示
========================================================*/
void LCD_ShowChar(uint16_t x, uint16_t y, uint8_t chr, uint8_t size, uint16_t mode)
{
    uint8_t temp, t, t1;
    uint16_t y0 = y;
    uint16_t i, col;
    uint8_t charWidth, charHigh, bytePerLine;
    const uint8_t *pFont = NULL;

    chr = chr - ' ';    /* 计算字库偏移 (从空格开始) */

    /* 根据字体大小选择对应的字库和参数 */
    if (size == 8)
    {
        pFont = &asc2_0806[chr * 6];
        charWidth  = 6;
        charHigh   = 8;
        bytePerLine = 1;
    }
    else if (size == 12)
    {
        pFont = &asc2_1206[chr * 12];
        charWidth  = 6;
        charHigh   = 12;
        bytePerLine = 1;
    }
    else if (size == 16)
    {
        pFont = &asc2_1608[chr * 16];
        charWidth  = 8;
        charHigh   = 16;
        bytePerLine = 1;
    }
    else if (size == 24)
    {
        pFont = &asc2_2412[chr * 36];
        charWidth  = 12;
        charHigh   = 24;
        bytePerLine = 2;        /* 24号字体: 12宽 = 2字节/行 */
    }
    else
    {
        return;     /* 不支持的字体大小 */
    }

    if (pFont == NULL) return;

    for (col = 0; col < charWidth; col++)
    {
        for (i = 0; i < bytePerLine; i++)
        {
            temp = pFont[col * bytePerLine + i];

            for (t1 = 0; t1 < 8; t1++)
            {
                if (temp & 0x80)
                {
                    LCD_DrawPoint(x + col, y, mode ? WHITE : BLACK);
                }
                else
                {
                    LCD_DrawPoint(x + col, y, mode ? BLACK : WHITE);
                }
                temp <<= 1;
                y++;
                if (y >= LCD_HEIGHT) return;
            }
        }
        y = y0;
    }
}

/*========================================================
    名称: LCD_ShowString
    作用: 在指定位置显示字符串
    参数: x, y - 左上角坐标
    参数: str - 字符串指针
    参数: size - 字体大小: 8, 12, 16, 24
    参数: mode - 0:反色显示, 1:正常显示
========================================================*/
void LCD_ShowString(uint16_t x, uint16_t y, const uint8_t *str,
                    uint8_t size, uint16_t mode)
{
    uint8_t charStep;

    /* 计算每个字符的显示宽度 */
    if (size == 8)
        charStep = 6;
    else if (size == 12)
        charStep = 6;
    else if (size == 16)
        charStep = 8;
    else if (size == 24)
        charStep = 12;
    else
        return;

    while (*str != '\0')
    {
        if (x + charStep > LCD_WIDTH)
        {
            x = 0;
            y += size;
            if (y + size > LCD_HEIGHT) break;
        }

        /* 判断是否为可打印字符 */
        if (*str < ' ' || *str > '~')
        {
            str++;
            x += charStep;
            continue;
        }

        LCD_ShowChar(x, y, *str, size, mode);
        x += charStep;
        str++;
    }
}

/*========================================================
    名称: LCD_ShowNum
    作用: 在指定位置显示整数
    参数: x, y - 左上角坐标
    参数: num - 要显示的数字
    参数: len - 数字位数
    参数: size - 字体大小
========================================================*/
void LCD_ShowNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size)
{
    uint8_t t, temp;
    uint8_t enshow = 0;
    uint8_t charStep;

    if (size == 8)
        charStep = 6;
    else if (size == 12)
        charStep = 6;
    else if (size == 16)
        charStep = 8;
    else if (size == 24)
        charStep = 12;
    else
        return;

    for (t = 0; t < len; t++)
    {
        temp = (num / LCD_Pow(10, len - t - 1)) % 10;

        if (enshow == 0 && t < (len - 1))
        {
            if (temp == 0)
            {
                LCD_ShowChar(x + t * charStep, y, ' ', size, 1);
                continue;
            }
            else
            {
                enshow = 1;
            }
        }
        LCD_ShowChar(x + t * charStep, y, temp + '0', size, 1);
    }
}

/*========================================================
    名称: LCD_ShowFloat
    作用: 显示浮点数
    参数: x, y - 左上角坐标
    参数: num - 浮点数
    参数: len - 总长度 (不含小数点)
    参数: size - 字体大小
    参数: decPlaces - 小数位数
========================================================*/
void LCD_ShowFloat(uint16_t x, uint16_t y, float num,
                   uint8_t len, uint8_t size, uint8_t decPlaces)
{
    uint8_t charStep;
    int32_t intPart;
    uint32_t decPart;
    uint8_t i;

    if (size == 8)
        charStep = 6;
    else if (size == 12)
        charStep = 6;
    else if (size == 16)
        charStep = 8;
    else if (size == 24)
        charStep = 12;
    else
        return;

    /* 分离整数和小数部分 */
    intPart = (int32_t)num;
    decPart = (uint32_t)((num - (float)intPart) * LCD_Pow(10, decPlaces) + 0.5f);

    /* 显示整数部分 */
    LCD_ShowNum(x, y, (uint32_t)intPart, len - decPlaces - 1, size);

    /* 小数点 */
    x += (len - decPlaces - 1) * charStep;
    LCD_ShowChar(x, y, '.', size, 1);
    x += charStep;

    /* 小数部分 */
    for (i = 0; i < decPlaces; i++)
    {
        uint8_t d = (decPart / LCD_Pow(10, decPlaces - 1 - i)) % 10;
        LCD_ShowChar(x + i * charStep, y, d + '0', size, 1);
    }
}

/*========================================================
    名称: LCD_ShowChinese
    作用: 显示一个12x12的中文字符
    参数: x, y  - 左上角坐标
    参数: index - 汉字在字库中的索引
    参数: mode  - 0:反色显示, 1:正常显示
    说明: 需要 lcdfont.h 中定义 chinese1212[] 字库
========================================================*/
void LCD_ShowChinese(uint16_t x, uint16_t y, uint8_t index, uint16_t mode)
{
    uint8_t i, j;
    uint8_t temp;
    uint16_t y0 = y;

    for (i = 0; i < 24; i++)
    {
        if (i < 12)
        {
            temp = chinese1212[2 * index * 12 + i];
        }
        else
        {
            temp = chinese1212[2 * index * 12 + i - 12 + 12];
        }

        for (j = 0; j < 8; j++)
        {
            if (temp & 0x80)
            {
                LCD_DrawPoint(x + j, y, mode ? WHITE : BLACK);
            }
            else
            {
                LCD_DrawPoint(x + j, y, mode ? BLACK : WHITE);
            }
            temp <<= 1;
        }
        y++;
        if ((y - y0) >= 12)
        {
            y = y0;
            x += 8;
        }
    }
}

/*========================================================
    名称: LCD_ShowPicture
    作用: 显示图片
    参数: x, y - 左上角坐标
    参数: w, h - 图片宽高(像素)
    参数: p   - 图片数据指针 (RGB565格式, 每个像素2字节)
========================================================*/
void LCD_ShowPicture(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                     const uint8_t *p)
{
    uint32_t i;
    uint32_t pixelCount = (uint32_t)w * (uint32_t)h;
    uint32_t byteCount = pixelCount * 2;

    if (x + w > LCD_WIDTH || y + h > LCD_HEIGHT) return;

    LCD_Address_Set(x, y, x + w - 1, y + h - 1);

    LCD_WriteBurstData(p, byteCount);
}

/*========================================================
    名称: LCD_BLK_Set
    作用: 背光开关控制
    参数: state - 0:关闭背光, 1:开启背光
========================================================*/
void LCD_BLK_Set(uint8_t state)
{
    if (state)
    {
        DL_GPIO_setPins(LCD_BLK_PORT, LCD_BLK_PIN);
    }
    else
    {
        DL_GPIO_clearPins(LCD_BLK_PORT, LCD_BLK_PIN);
    }
}

/*========================================================
    名称: LCD_printf
    作用: 使用printf格式化字符串在LCD上显示
    参数: x, y   - 左上角坐标
    参数: size   - 字体大小: 8, 12, 16, 24
    参数: format - 格式化字符串
    参数: ...    - 可变参数列表
========================================================*/
void LCD_printf(uint16_t x, uint16_t y, uint8_t size, const char *format, ...)
{
    char string[64];
    va_list arg;

    va_start(arg, format);
    vsprintf(string, format, arg);
    va_end(arg);

    LCD_ShowString(x, y, (uint8_t *)string, size, 1);
}
