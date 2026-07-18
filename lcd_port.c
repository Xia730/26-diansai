#include "board.h"

#include <stdint.h>
#include <stddef.h>

/*
 * LCD Chip Select 控制
 * level = 1: 释放 CS
 * level = 0: 拉低 CS，选中 LCD
 */
void lcd_set_cs(uint8_t level)
{
    if (level) {
        DL_GPIO_setPins(GPIOB, SPI_LCD1_CS_PIN);
    } else {
        DL_GPIO_clearPins(GPIOB, SPI_LCD1_CS_PIN);
    }
}

/*
 * LCD 数据/命令选择引脚控制
 * level = 1: 数据模式
 * level = 0: 命令模式
 */
void lcd_set_dc(uint8_t level)
{
    if (level) {
        DL_GPIO_setPins(GPIOB, SPI_LCD1_DC_PIN);
    } else {
        DL_GPIO_clearPins(GPIOB, SPI_LCD1_DC_PIN);
    }
}

/*
 * LCD 复位引脚控制
 * level = 1: 释放复位
 * level = 0: 保持复位
 */
void lcd_set_rst(uint8_t level)
{
    if (level) {
        DL_GPIO_setPins(GPIOB, SPI_LCD1_RES_PIN);
    } else {
        DL_GPIO_clearPins(GPIOB, SPI_LCD1_RES_PIN);
    }
}

/*
 * LCD 背光控制
 * level = 1: 打开背光
 * level = 0: 关闭背光
 */
void lcd_set_bl(uint8_t level)
{
    if (level) {
        DL_GPIO_setPins(GPIOB, SPI_LCD1_BLK_PIN);
    } else {
        DL_GPIO_clearPins(GPIOB, SPI_LCD1_BLK_PIN);
    }
}

/*
 * 毫秒级延时
 * 使用 board 里的延时接口，避免写死 CPU 频率。
 */
void lcd_delay_ms(uint32_t ms)
{
    delay_ms(ms);
}

void lcd_spi_write(const uint8_t *buf, size_t len)
{
    while (len--) {
        while (DL_SPI_isBusy(SPI_LCD_INST)) {
        }
        DL_SPI_transmitData8(SPI_LCD_INST, *buf++);
    }

    while (DL_SPI_isBusy(SPI_LCD_INST)) {
    }
}
