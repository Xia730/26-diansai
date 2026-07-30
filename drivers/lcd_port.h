/**
 * @file    lcd_port.h
 * @brief   ST7789 LCD 底层驱动 — 引脚定义、SPI+DMA API
 * @note    命令/寄存器走轮询，像素数据走 DMA
 */
#ifndef __LCD_PORT_H
#define __LCD_PORT_H

#include "board.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========== 显示方向 ========== */
#define USE_HORIZONTAL  1   /* 0/1=竖屏240x240  2/3=横屏240x240 (方屏) */

#define LCD_W  240
#define LCD_H  240

/* ========== 引脚宏 (全部在 GPIOB) ========== */
#define LCD_RES_Clr()  DL_GPIO_clearPins(SPI_LCD1_PORT, SPI_LCD1_RES_PIN)
#define LCD_RES_Set()  DL_GPIO_setPins(SPI_LCD1_PORT, SPI_LCD1_RES_PIN)
#define LCD_DC_Clr()   DL_GPIO_clearPins(SPI_LCD1_PORT, SPI_LCD1_DC_PIN)
#define LCD_DC_Set()   DL_GPIO_setPins(SPI_LCD1_PORT, SPI_LCD1_DC_PIN)
#define LCD_CS_Clr()   DL_GPIO_clearPins(SPI_LCD1_PORT, SPI_LCD1_CS_PIN)
#define LCD_CS_Set()   DL_GPIO_setPins(SPI_LCD1_PORT, SPI_LCD1_CS_PIN)
#define LCD_BLK_Clr()  DL_GPIO_clearPins(SPI_LCD1_PORT, SPI_LCD1_BLK_PIN)
#define LCD_BLK_Set()  DL_GPIO_setPins(SPI_LCD1_PORT, SPI_LCD1_BLK_PIN)

/* ========== DMA ========== */
#define LCD_DMA_BUF_SIZE  1024  /* 512像素/块 (每像素2字节) */

/* ========== 底层 API ========== */

/* 命令/寄存器 (轮询) */
void LCD_WR_REG(uint8_t cmd);
void LCD_WR_DATA8(uint8_t dat);
void LCD_WR_DATA16(uint16_t dat);

/* 窗口设置 */
void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

/* 硬件初始化 */
void LCD_Init(void);

/* DMA 批量像素写入 */
void LCD_WR_DATA_DMA(const uint16_t *data, uint32_t pixel_count);

/* 等待 DMA 完成 (当前同步实现, 保留接口) */
void LCD_DMA_WaitComplete(void);

#ifdef __cplusplus
}
#endif

#endif /* __LCD_PORT_H */
