/**
 * @file    lcd.h
 * @brief   ST7735 液晶屏驱动接口头文件
 * 
 * 定义了屏幕尺寸、常用颜色、偏移量宏以及对外提供的绘图、控制函数。
 * 颜色宏使用 RGB565 格式（16 位色深），可直接用于 lcd_fill_color、lcd_draw_string 等函数。
 */

#ifndef LCD_H
#define LCD_H

#include <stddef.h>
#include <stdint.h>

/* ================================================================
 * 屏幕物理参数
 * ================================================================ */
#define LCD_WIDTH      128   /**< 液晶屏水平像素数（X 方向） */
#define LCD_HEIGHT     160   /**< 液晶屏垂直像素数（Y 方向） */

/* ================================================================
 * 常用颜色宏（RGB565 格式）
 * ================================================================ */
#define BLACK          0x0000 /**< 纯黑   (R:0   G:0   B:0  ) */
#define NAVY           0x000F /**< 藏青   (R:0   G:0   B:128) */
#define DARKGREEN      0x03E0 /**< 深绿   (R:0   G:128 B:0  ) */
#define DARKCYAN       0x03EF /**< 深青   (R:0   G:128 B:128) */
#define MAROON         0x7800 /**< 栗色   (R:128 G:0   B:0  ) */
#define PURPLE         0x780F /**< 紫色   (R:128 G:0   B:128) */
#define OLIVE          0x7BE0 /**< 橄榄   (R:128 G:128 B:0  ) */
#define LIGHTGREY      0xC618 /**< 浅灰   (R:192 G:192 B:192) */
#define DARKGREY       0x7BEF /**< 深灰   (R:128 G:128 B:128) */
#define BLUE           0x001F /**< 蓝色   (R:0   G:0   B:255) */
#define GREEN          0x07E0 /**< 绿色   (R:0   G:255 B:0  ) */
#define CYAN           0x07FF /**< 青色   (R:0   G:255 B:255) */
#define RED            0xF800 /**< 红色   (R:255 G:0   B:0  ) */
#define MAGENTA        0xF81F /**< 品红   (R:255 G:0   B:255) */
#define YELLOW         0xFFE0 /**< 黄色   (R:255 G:255 B:0  ) */
#define WHITE          0xFFFF /**< 白色   (R:255 G:255 B:255) */
#define ORANGE         0xFD20 /**< 橙色   (R:255 G:165 B:0  ) */
#define GREENYELLOW    0xAFE5 /**< 黄绿   (R:173 G:255 B:47 ) */
#define PINK           0xF81F /**< 粉红   (R:255 G:0   B:255) */

/* ================================================================
 * 面板偏移量（可根据实际屏幕微调）
 * ================================================================ */
#define LCD_X_OFFSET   0      /**< 列地址偏移（0 = 无偏移） */
#define LCD_Y_OFFSET   0      /**< 行地址偏移（0 = 无偏移） */

/* ================================================================
 * 基础通信接口函数
 * ================================================================ */

/**
 * @brief  向 LCD 控制器发送一个命令字节
 * @param  cmd 命令码（见 ST7735 命令列表）
 */
void lcd_write_command(uint8_t cmd);

/**
 * @brief  向 LCD 控制器发送单个数据字节
 * @param  data 数据字节
 */
void lcd_write_data8(uint8_t data);

/**
 * @brief  向 LCD 控制器发送多个数据字节
 * @param  data 数据缓冲区指针
 * @param  len  数据长度（字节数）
 */
void lcd_write_data(const uint8_t *data, size_t len);

/* ================================================================
 * 硬件控制函数
 * ================================================================ */

/**
 * @brief  硬件复位液晶屏（通过 RST 引脚）
 */
void lcd_reset(void);

/**
 * @brief  开启背光
 */
void lcd_backlight_on(void);

/**
 * @brief  关闭背光
 */
void lcd_backlight_off(void);

/* ================================================================
 * 绘图相关函数
 * ================================================================ */

/**
 * @brief  设置后续绘图操作的窗口区域（列、行范围）
 * @param  x0 起始列（包含，0 ~ LCD_WIDTH - 1）
 * @param  y0 起始行（包含，0 ~ LCD_HEIGHT - 1）
 * @param  x1 结束列（包含）
 * @param  y1 结束行（包含）
 * @note   坐标会自动加上 LCD_X_OFFSET / LCD_Y_OFFSET 偏移量。
 *         调用后发送数据将按自增顺序填充该矩形。
 */
void lcd_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

/**
 * @brief  用单一颜色填充整个屏幕
 * @param  color RGB565 格式颜色值（可使用预定义宏如 RED、GREEN 等）
 */
void lcd_fill_color(uint16_t color);

/**
 * @brief  绘制一个像素点
 * @param  x     横坐标（0 ~ LCD_WIDTH - 1）
 * @param  y     纵坐标（0 ~ LCD_HEIGHT - 1）
 * @param  color RGB565 颜色值
 */
void lcd_draw_pixel(uint16_t x, uint16_t y, uint16_t color);

/**
 * @brief  在指定位置绘制一个 8x8 点阵字符（使用内置 5x7 字体）
 * @param  x   左上角横坐标
 * @param  y   左上角纵坐标
 * @param  c   要显示的 ASCII 字符
 * @param  fg  前景色（字体颜色，RGB565）
 * @param  bg  背景色（RGB565）
 * @note   字符尺寸固定为 8x8 像素，字模实际宽 5 列、高 7 行，左右各留 1 像素边距。
 *         坐标超出屏幕范围时不进行绘制。
 */
void lcd_draw_char(uint16_t x, uint16_t y, char c, uint16_t fg, uint16_t bg);

/**
 * @brief  在指定位置绘制字符串
 * @param  x   起始横坐标
 * @param  y   起始纵坐标
 * @param  str 要显示的字符串（以 '\0' 结尾）
 * @param  fg  前景色
 * @param  bg  背景色
 * @note   支持 '\n' 换行（换行后光标回到 x，y 增加一个字符高度）。
 *         无自动折行功能，超出屏幕右边界的内容会被裁剪。
 */
void lcd_draw_string(uint16_t x, uint16_t y, const char *str, uint16_t fg, uint16_t bg);

// 需要完整包含 <stdio.h>（如果头文件中未包含，可让用户在使用前包含）
#include <stdio.h>

/**
 * @brief  向屏幕格式化输出字符串（类似 printf，函数封装）
 * @param  x       X 坐标（像素）
 * @param  y       Y 坐标（像素）
 * @param  fg      前景色 (RGB565)
 * @param  bg      背景色 (RGB565)
 * @param  format  格式化字符串（同 printf）
 * @param  ...     可变参数
 * @note   内部缓冲区固定 64 字节；若平台栈对齐有问题，
 *         可在函数前加 __attribute__((force_align_arg_pointer))
 */
void lcd_printf(uint16_t x, uint16_t y, uint16_t fg, uint16_t bg,
                const char *format, ...);

/* ================================================================
 * 初始化函数
 * ================================================================ */

void lcd_init(void);

#endif /* LCD_H */