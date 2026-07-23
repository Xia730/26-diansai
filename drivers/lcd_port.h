/**
 * @file    lcd_port.h
 * @brief   LCD 硬件抽象层接口声明
 * 
 * 定义了与硬件平台相关的底层操作函数，包括 GPIO 控制（CS、DC、RST、BL）、
 * SPI 数据发送以及毫秒级延时。这些函数需在 board.c（或等效平台文件）中实现。
 */

#ifndef LCD_PORT_H
#define LCD_PORT_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief  控制 LCD 片选引脚（CS）
 * @param  level 电平状态：1 = 释放（高电平），0 = 选中（低电平）
 */
void lcd_set_cs(uint8_t level);

/**
 * @brief  控制 LCD 数据/命令选择引脚（DC）
 * @param  level 电平状态：1 = 数据模式，0 = 命令模式
 */
void lcd_set_dc(uint8_t level);

/**
 * @brief  控制 LCD 硬件复位引脚（RST）
 * @param  level 电平状态：1 = 释放复位（高电平），0 = 保持复位（低电平）
 */
void lcd_set_rst(uint8_t level);

/**
 * @brief  控制 LCD 背光引脚（BL）
 * @param  level 电平状态：1 = 打开背光，0 = 关闭背光
 */
void lcd_set_bl(uint8_t level);

/**
 * @brief  毫秒级延时
 * @param  ms 延时的毫秒数
 */
void lcd_delay_ms(uint32_t ms);

/**
 * @brief  通过 SPI 发送数据块
 * @param  buf 数据缓冲区指针
 * @param  len 要发送的字节数
 * @note   该函数应阻塞直到所有数据发送完成。
 *         可基于硬件 SPI 或软件模拟 SPI 实现。
 */
void lcd_spi_write(const uint8_t *buf, size_t len);

#endif /* LCD_PORT_H */