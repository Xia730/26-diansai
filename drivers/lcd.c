/**
 * @file    lcd.c
 * @brief   ST7735 0.96/1.8寸 TFT LCD 驱动实现
 * 
 * 依赖 board.h 中提供的硬件抽象接口：
 *   lcd_set_cs()    - 片选控制
 *   lcd_set_dc()    - 数据/命令选择
 *   lcd_set_rst()   - 复位控制
 *   lcd_set_bl()    - 背光控制
 *   lcd_spi_write() - SPI 发送
 *   lcd_delay_ms()  - 毫秒延时
 */

#include "lcd.h"
#include "board.h"
#include "lcd_front.h"        /* 引入分离的字库模块 */
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

/* ---------- 硬件抽象层外部函数声明 ---------- */
extern void lcd_set_cs(uint8_t level);
extern void lcd_set_dc(uint8_t level);
extern void lcd_set_rst(uint8_t level);
extern void lcd_set_bl(uint8_t level);
extern void lcd_spi_write(const uint8_t *buf, size_t len);
extern void lcd_delay_ms(uint32_t ms);

/* ---------- ST7735 命令定义 ---------- */
#define ST7735_SWRESET   0x01  // 软件复位
#define ST7735_SLPOUT    0x11  // 退出睡眠模式
#define ST7735_DISPON    0x29  // 显示开启
#define ST7735_CASET     0x2A  // 列地址设置
#define ST7735_RASET     0x2B  // 行地址设置
#define ST7735_RAMWR     0x2C  // 内存写入
#define ST7735_COLMOD    0x3A  // 像素格式设置（接口颜色格式）
#define ST7735_MADCTL    0x36  // 内存数据访问控制（方向/颜色顺序）
#define ST7735_FRMCTR1   0xB1  // 帧速率控制1（正常模式）
#define ST7735_FRMCTR2   0xB2  // 帧速率控制2（空闲模式）
#define ST7735_FRMCTR3   0xB3  // 帧速率控制3（部分模式）
#define ST7735_INVCTR    0xB4  // 显示反转控制
#define ST7735_PWCTR1    0xC0  // 电源控制1
#define ST7735_PWCTR2    0xC1  // 电源控制2
#define ST7735_PWCTR3    0xC2  // 电源控制3（正常模式）
#define ST7735_PWCTR4    0xC3  // 电源控制4（空闲模式）
#define ST7735_PWCTR5    0xC4  // 电源控制5（部分模式）
#define ST7735_VMCTR1    0xC5  // VCOM 控制1
#define ST7735_INVOFF    0x20  // 显示反转关闭
#define ST7735_NORON     0x13  // 正常显示模式开启
#define ST7735_GMCTRP1   0xE0  // Gamma 校正（正极性）
#define ST7735_GMCTRN1   0xE1  // Gamma 校正（负极性）

/* ---------- 内置字库常量 ---------- */
#define LCD_CHAR_W 8   // 字符宽度（像素）
#define LCD_CHAR_H 8   // 字符高度（像素）

/* ================================================================
 * 内部辅助函数
 * ================================================================ */

/**
 * @brief  向 LCD 写入一个 16 位数据（通常为颜色值或坐标）
 * @param  value 要发送的 16 位数据
 * @note   先发送高字节，再发送低字节
 */
static void lcd_write_u16(uint16_t value)
{
    uint8_t data[2];

    data[0] = (uint8_t)(value >> 8);
    data[1] = (uint8_t)(value & 0xFF);
    lcd_write_data(data, 2);
}

/* ================================================================
 * 基础写操作（命令/数据）
 * ================================================================ */

/**
 * @brief  向 LCD 发送一个命令字节
 * @param  cmd 命令码
 * @note   操作顺序：设置 DC=0（命令模式）-> 片选拉低 -> SPI 发送 -> 片选释放
 */
void lcd_write_command(uint8_t cmd)
{
    lcd_set_dc(0);
    lcd_set_cs(0);
    lcd_spi_write(&cmd, 1);
    lcd_set_cs(1);
}

/**
 * @brief  向 LCD 发送一个数据字节（单字节）
 * @param  data 数据字节
 */
void lcd_write_data8(uint8_t data)
{
    lcd_set_dc(1);
    lcd_set_cs(0);
    lcd_spi_write(&data, 1);
    lcd_set_cs(1);
}

/**
 * @brief  向 LCD 发送一块数据（多字节）
 * @param  data 数据缓冲区指针
 * @param  len  数据长度（字节）
 */
void lcd_write_data(const uint8_t *data, size_t len)
{
    if (data == NULL || len == 0) {
        return;
    }

    lcd_set_dc(1);
    lcd_set_cs(0);
    lcd_spi_write(data, len);
    lcd_set_cs(1);
}

/* ================================================================
 * 硬件控制（复位、背光）
 * ================================================================ */

/**
 * @brief  LCD 硬件复位序列
 * @note   拉低 RST 至少 10ms，然后拉高并等待 120ms 让模块稳定
 */
void lcd_reset(void)
{
    lcd_set_rst(0);
    lcd_delay_ms(10);
    lcd_set_rst(1);
    lcd_delay_ms(120);
}

/**
 * @brief  打开背光
 */
void lcd_backlight_on(void)
{
    lcd_set_bl(1);
}

/**
 * @brief  关闭背光
 */
void lcd_backlight_off(void)
{
    lcd_set_bl(0);
}

/* ================================================================
 * 绘图辅助
 * ================================================================ */

/**
 * @brief  设置绘图窗口（列/行范围）
 * @param  x0 起始列（包含）
 * @param  y0 起始行（包含）
 * @param  x1 结束列（包含）
 * @param  y1 结束行（包含）
 * @note   坐标会自动加上偏移量 LCD_X_OFFSET / LCD_Y_OFFSET
 *         然后发送 CASET、RASET、RAMWR 命令，后续写入数据将填充该矩形区域
 */
void lcd_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    uint16_t xs = x0 + LCD_X_OFFSET;
    uint16_t xe = x1 + LCD_X_OFFSET;
    uint16_t ys = y0 + LCD_Y_OFFSET;
    uint16_t ye = y1 + LCD_Y_OFFSET;

    lcd_write_command(ST7735_CASET);
    lcd_write_u16(xs);
    lcd_write_u16(xe);

    lcd_write_command(ST7735_RASET);
    lcd_write_u16(ys);
    lcd_write_u16(ye);

    lcd_write_command(ST7735_RAMWR);
}

/**
 * @brief  全屏填充单一颜色
 * @param  color RGB565 格式颜色值
 * @note   预先生成一行的颜色数据（128*2 字节），然后逐行写入以提高效率
 */
void lcd_fill_color(uint16_t color)
{
    uint8_t line[LCD_WIDTH * 2];

    /* 填充一行数据 */
    for (uint16_t i = 0; i < LCD_WIDTH * 2; i += 2) {
        line[i] = (uint8_t)(color >> 8);
        line[i + 1] = (uint8_t)(color & 0xFF);
    }

    lcd_set_window(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);

    /* 逐行发送 */
    for (uint16_t y = 0; y < LCD_HEIGHT; y++) {
        lcd_write_data(line, sizeof(line));
    }
}

/**
 * @brief  绘制单个像素
 * @param  x     横坐标（0 ~ LCD_WIDTH-1）
 * @param  y     纵坐标（0 ~ LCD_HEIGHT-1）
 * @param  color RGB565 颜色
 */
void lcd_draw_pixel(uint16_t x, uint16_t y, uint16_t color)
{
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) {
        return;
    }

    lcd_set_window(x, y, x, y);
    lcd_write_u16(color);
}

/* ================================================================
 * 字符 / 字符串绘制（字库来自 font.c / font.h）
 * ================================================================ */

/**
 * @brief  在指定位置绘制一个字符
 * @param  x   左上角横坐标
 * @param  y   左上角纵坐标
 * @param  c   要显示的字符
 * @param  fg  前景色（字体颜色，RGB565）
 * @param  bg  背景色（RGB565）
 * @note   字符尺寸为 LCD_CHAR_W x LCD_CHAR_H（默认8x8）
 *         字模宽度实际为5列，左右各留1像素边距，高度7行，下方留1行
 */
void lcd_draw_char(uint16_t x, uint16_t y, char c, uint16_t fg, uint16_t bg)
{
    const uint8_t *glyph;
    uint8_t col;
    uint8_t row;

    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) {
        return;
    }

    glyph = lcd_get_glyph(c);   /* 调用 font.c 中的查找函数 */

    /* 逐行扫描绘制 */
    for (row = 0; row < LCD_CHAR_H; row++) {
        for (col = 0; col < LCD_CHAR_W; col++) {
            uint16_t px = (uint16_t)(x + col);
            uint16_t py = (uint16_t)(y + row);
            uint16_t color = bg;

            if (px >= LCD_WIDTH || py >= LCD_HEIGHT) {
                continue;
            }

            /* 有效像素区域：列 1~5，行 0~6 */
            if (col >= 1 && col <= 5 && row < 7) {
                if (glyph[col - 1] & (1U << row)) {
                    color = fg;   // 字模对应位为1，用前景色
                }
            }

            lcd_draw_pixel(px, py, color);
        }
    }
}

/**
 * @brief  在指定位置绘制字符串（自动换行需手动处理 '\n'）
 * @param  x   起始横坐标
 * @param  y   起始纵坐标
 * @param  str 要显示的字符串（以 \0 结尾）
 * @param  fg  前景色
 * @param  bg  背景色
 * @note   支持 '\n' 换行，换行后光标回到 x 起始位置，y 下移一个字符高度
 *         不支持自动折行，超出屏幕右边界会被裁剪（lcd_draw_char 内有边界检测）
 */
void lcd_draw_string(uint16_t x, uint16_t y, const char *str, uint16_t fg, uint16_t bg)
{
    uint16_t cursor_x;

    if (str == NULL) {
        return;
    }

    cursor_x = x;

    while (*str) {
        if (*str == '\n') {
            /* 换行：y 下移一个字符高度，x 回到起始位置 */
            y = (uint16_t)(y + LCD_CHAR_H);
            cursor_x = x;
        } else {
            lcd_draw_char(cursor_x, y, *str, fg, bg);
            cursor_x = (uint16_t)(cursor_x + LCD_CHAR_W);
        }
        str++;
    }
}


/**
 * @brief  屏幕格式化输出（可变参数函数）
 *         要求调用时 SP 保持 8 字节对齐（AAPCS 标准）
 */
void lcd_printf(uint16_t x, uint16_t y, uint16_t fg, uint16_t bg,
                const char *format, ...)
{
    char buf[64];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);

    lcd_draw_string(x, y, buf, fg, bg);
}

/* ================================================================
 * LCD 初始化序列（针对 ST7735 常见模组）
 * ================================================================ */

/**
 * @brief  ST7735 初始化
 * @note   包含软件复位、退出睡眠、设置像素格式、显示方向、
 *          帧速率、电源设置、Gamma 校正、最后开启显示和背光。
 *         如果屏幕方向/颜色不对，可以调整 MADCTL 寄存器值（0x60 / 0x68）。
 */
void lcd_init(void)
{
    uint8_t data[16];

    /* 引脚初始状态：CS 高（未选中），DC 命令模式，RST 高（不复位），背光关闭 */
    lcd_set_cs(1);
    lcd_set_dc(0);
    lcd_set_rst(1);
    lcd_set_bl(0);

    /* 硬件复位 */
    lcd_reset();

    /* 软件复位 */
    lcd_write_command(ST7735_SWRESET);
    lcd_delay_ms(120);

    /* 退出睡眠模式 */
    lcd_write_command(ST7735_SLPOUT);
    lcd_delay_ms(120);

    /* 设置像素格式：16位/像素（RGB565） */
    lcd_write_command(ST7735_COLMOD);
    lcd_write_data8(0x05);
    lcd_delay_ms(10);

    /* 显示方向控制：MV=1, MX=1, MY=0，RGB顺序不变 -> 横屏模式 */
    lcd_write_command(ST7735_MADCTL);
    lcd_write_data8(0x60);   // 横屏，若颜色顺序不对可尝试 0x68

    /* 帧速率控制（正常模式） */
    lcd_write_command(ST7735_FRMCTR1);
    data[0] = 0x01;
    data[1] = 0x2C;
    data[2] = 0x2D;
    lcd_write_data(data, 3);

    /* 帧速率控制（空闲模式） */
    lcd_write_command(ST7735_FRMCTR2);
    data[0] = 0x01;
    data[1] = 0x2C;
    data[2] = 0x2D;
    lcd_write_data(data, 3);

    /* 帧速率控制（部分模式） */
    lcd_write_command(ST7735_FRMCTR3);
    data[0] = 0x01;
    data[1] = 0x2C;
    data[2] = 0x2D;
    data[3] = 0x01;
    data[4] = 0x2C;
    data[5] = 0x2D;
    lcd_write_data(data, 6);

    /* 显示反转控制 */
    lcd_write_command(ST7735_INVCTR);
    lcd_write_data8(0x07);

    /* 电源控制设置 */
    lcd_write_command(ST7735_PWCTR1);
    data[0] = 0xA2;
    data[1] = 0x02;
    data[2] = 0x84;
    lcd_write_data(data, 3);

    lcd_write_command(ST7735_PWCTR2);
    lcd_write_data8(0xC5);

    lcd_write_command(ST7735_PWCTR3);
    data[0] = 0x0A;
    data[1] = 0x00;
    lcd_write_data(data, 2);

    lcd_write_command(ST7735_PWCTR4);
    data[0] = 0x8A;
    data[1] = 0x2A;
    lcd_write_data(data, 2);

    lcd_write_command(ST7735_PWCTR5);
    data[0] = 0x8A;
    data[1] = 0xEE;
    lcd_write_data(data, 2);

    /* VCOM 控制 */
    lcd_write_command(ST7735_VMCTR1);
    lcd_write_data8(0x0E);

    /* 关闭反转（即正常显示） */
    lcd_write_command(ST7735_INVOFF);

    /* Gamma 校正（正极性） */
    lcd_write_command(ST7735_GMCTRP1);
    data[0]  = 0x02;
    data[1]  = 0x1C;
    data[2]  = 0x07;
    data[3]  = 0x12;
    data[4]  = 0x37;
    data[5]  = 0x32;
    data[6]  = 0x29;
    data[7]  = 0x2D;
    data[8]  = 0x29;
    data[9]  = 0x25;
    data[10] = 0x2B;
    data[11] = 0x39;
    data[12] = 0x00;
    data[13] = 0x01;
    data[14] = 0x03;
    data[15] = 0x10;
    lcd_write_data(data, 16);

    /* Gamma 校正（负极性） */
    lcd_write_command(ST7735_GMCTRN1);
    data[0]  = 0x03;
    data[1]  = 0x1D;
    data[2]  = 0x07;
    data[3]  = 0x06;
    data[4]  = 0x2E;
    data[5]  = 0x2C;
    data[6]  = 0x29;
    data[7]  = 0x2D;
    data[8]  = 0x2E;
    data[9]  = 0x2E;
    data[10] = 0x37;
    data[11] = 0x3F;
    data[12] = 0x00;
    data[13] = 0x00;
    data[14] = 0x02;
    data[15] = 0x10;
    lcd_write_data(data, 16);

    /* 进入正常显示模式 */
    lcd_write_command(ST7735_NORON);
    lcd_delay_ms(10);

    /* 打开显示 */
    lcd_write_command(ST7735_DISPON);
    lcd_delay_ms(120);

    /* 点亮背光 */
    lcd_backlight_on();
}