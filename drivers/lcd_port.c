/**
 * @file    lcd_port.c
 * @brief   ST7735 LCD 底层 — SPI 轮询命令 + DMA 批量像素
 * @note    SysConfig 已生成: SPI_LCD_INST(SPI1), DMA_LCD_TX_CHAN_ID(0)
 */
#include "lcd_port.h"
#include "lcd.h"   /* for LCD_Fill in LCD_Init */

/* ========== 内部: SPI 单字节写 (轮询, CS 自动 toggle) ========== */
static void _LCD_Writ_Bus(uint8_t dat)
{
    LCD_CS_Clr();
    DL_SPI_transmitData8(SPI_LCD_INST, dat);
    while (DL_SPI_isBusy(SPI_LCD_INST));
    (void)DL_SPI_receiveData8(SPI_LCD_INST);  /* 读空 RX FIFO */
    while (DL_SPI_isBusy(SPI_LCD_INST));
    LCD_CS_Set();
}

/* ========== 命令 / 寄存器 (轮询) ========== */

void LCD_WR_REG(uint8_t cmd)
{
    LCD_DC_Clr();
    _LCD_Writ_Bus(cmd);
    LCD_DC_Set();   /* 恢复到数据模式 */
}

void LCD_WR_DATA8(uint8_t dat)
{
    _LCD_Writ_Bus(dat);
}

void LCD_WR_DATA16(uint16_t dat)
{
    _LCD_Writ_Bus((uint8_t)(dat >> 8));
    _LCD_Writ_Bus((uint8_t)(dat));
}

/* ========== 窗口设置 ========== */

void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    uint8_t x_offset, y_offset;

    switch (USE_HORIZONTAL) {
        case 0: x_offset = 2; y_offset = 1; break;
        case 1: x_offset = 2; y_offset = 1; break;
        case 2: x_offset = 1; y_offset = 2; break;
        default: x_offset = 1; y_offset = 2; break;
    }

    LCD_WR_REG(0x2A);  /* CASET 列地址 */
    LCD_WR_DATA16(x1 + x_offset);
    LCD_WR_DATA16(x2 + x_offset);

    LCD_WR_REG(0x2B);  /* RASET 行地址 */
    LCD_WR_DATA16(y1 + y_offset);
    LCD_WR_DATA16(y2 + y_offset);

    LCD_WR_REG(0x2C);  /* RAMWR 开始写 GRAM */
}

/* ========== DMA 批量像素写入 ========== */

/**
 * @brief  内部 — 启动 DMA 传输并同步等待完成
 * @param  byte_data  源字节 buffer (静态/全局)
 * @param  byte_count 字节数
 * @note   CS 在整个 DMA burst 期间保持低电平
 */
static void _LCD_DMA_Start(const uint8_t *byte_data, uint32_t byte_count)
{
    if (byte_count == 0) return;

    LCD_CS_Clr();

    /* SINGLE_BLOCK: 设参数, 重新使能 (通道已 auto-disable) */
    DL_DMA_setSrcAddr(DMA, DMA_LCD_TX_CHAN_ID, (uint32_t)byte_data);
    DL_DMA_setDestAddr(DMA, DMA_LCD_TX_CHAN_ID,
                       (uint32_t)(&SPI_LCD_INST->TXDATA));
    DL_DMA_setTransferSize(DMA, DMA_LCD_TX_CHAN_ID, byte_count);
    DL_DMA_enableChannel(DMA, DMA_LCD_TX_CHAN_ID);

    /* 轮询 DMA 计数 → 0 */
    while (DL_DMA_getTransferSize(DMA, DMA_LCD_TX_CHAN_ID) > 0) {}

    /* 等待 SPI shift register 排空 */
    while (DL_SPI_isBusy(SPI_LCD_INST)) {}

    LCD_CS_Set();
}

void LCD_WR_DATA_DMA(const uint16_t *data, uint32_t pixel_count)
{
    /* 每个像素 2 字节 (MSB first), 小端 CPU 需注意字节序 */
    _LCD_DMA_Start((const uint8_t *)data, pixel_count * 2);
}

void LCD_DMA_WaitComplete(void)
{
    /* 当前 DMA 为同步实现，此函数作为将来异步扩展的接口保留 */
}

/* ========== 硬件初始化 ========== */

void LCD_Init(void)
{
    /* ---- 硬件复位 ---- */
    LCD_RES_Clr();
    delay_ms(100);
    LCD_RES_Set();
    delay_ms(100);

    /* ---- ST7735 初始化序列 ---- */

    LCD_WR_REG(0x11);   /* 退出睡眠 */
    delay_ms(120);

    /* 帧率控制 */
    LCD_WR_REG(0xB1);
    LCD_WR_DATA8(0x05); LCD_WR_DATA8(0x3C); LCD_WR_DATA8(0x3C);

    LCD_WR_REG(0xB2);
    LCD_WR_DATA8(0x05); LCD_WR_DATA8(0x3C); LCD_WR_DATA8(0x3C);

    LCD_WR_REG(0xB3);
    LCD_WR_DATA8(0x05); LCD_WR_DATA8(0x3C); LCD_WR_DATA8(0x3C);
    LCD_WR_DATA8(0x05); LCD_WR_DATA8(0x3C); LCD_WR_DATA8(0x3C);

    LCD_WR_REG(0xB4);   /* 点反转 */
    LCD_WR_DATA8(0x03);

    /* 电源设置 */
    LCD_WR_REG(0xC0);
    LCD_WR_DATA8(0x28); LCD_WR_DATA8(0x08); LCD_WR_DATA8(0x04);

    LCD_WR_REG(0xC1);
    LCD_WR_DATA8(0xC0);

    LCD_WR_REG(0xC2);
    LCD_WR_DATA8(0x0D); LCD_WR_DATA8(0x00);

    LCD_WR_REG(0xC3);
    LCD_WR_DATA8(0x8D); LCD_WR_DATA8(0x2A);

    LCD_WR_REG(0xC4);
    LCD_WR_DATA8(0x8D); LCD_WR_DATA8(0xEE);

    LCD_WR_REG(0xC5);   /* VCOM */
    LCD_WR_DATA8(0x1A);

    /* 方向控制 (MADCTL) */
    LCD_WR_REG(0x36);
    switch (USE_HORIZONTAL) {
        case 0: LCD_WR_DATA8(0x00); break;
        case 1: LCD_WR_DATA8(0xC0); break;
        case 2: LCD_WR_DATA8(0x70); break;
        default: LCD_WR_DATA8(0xA0); break;
    }

    /* Gamma 正极性 */
    LCD_WR_REG(0xE0);
    LCD_WR_DATA8(0x04); LCD_WR_DATA8(0x22); LCD_WR_DATA8(0x07);
    LCD_WR_DATA8(0x0A); LCD_WR_DATA8(0x2E); LCD_WR_DATA8(0x30);
    LCD_WR_DATA8(0x25); LCD_WR_DATA8(0x2A); LCD_WR_DATA8(0x28);
    LCD_WR_DATA8(0x26); LCD_WR_DATA8(0x2E); LCD_WR_DATA8(0x3A);
    LCD_WR_DATA8(0x00); LCD_WR_DATA8(0x01); LCD_WR_DATA8(0x03);
    LCD_WR_DATA8(0x13);

    /* Gamma 负极性 */
    LCD_WR_REG(0xE1);
    LCD_WR_DATA8(0x04); LCD_WR_DATA8(0x16); LCD_WR_DATA8(0x06);
    LCD_WR_DATA8(0x0D); LCD_WR_DATA8(0x2D); LCD_WR_DATA8(0x26);
    LCD_WR_DATA8(0x23); LCD_WR_DATA8(0x27); LCD_WR_DATA8(0x27);
    LCD_WR_DATA8(0x25); LCD_WR_DATA8(0x2D); LCD_WR_DATA8(0x3B);
    LCD_WR_DATA8(0x00); LCD_WR_DATA8(0x01); LCD_WR_DATA8(0x04);
    LCD_WR_DATA8(0x13);

    /* 像素格式: 16-bit (RGB565) */
    LCD_WR_REG(0x3A);
    LCD_WR_DATA8(0x05);

    /* 开显示 */
    LCD_WR_REG(0x29);

    /* ---- 清屏 (背光开启前先刷黑，避免显示残影) ---- */
    LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);

    /* ---- 背光开 ---- */
    LCD_BLK_Set();
		
}
