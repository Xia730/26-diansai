#ifndef __LCD_INIT_H
#define __LCD_INIT_H

#include "board.h"

/*========================================================
    ST7735 硬件引脚定义 (硬件SPI + GPIO控制线)

    SPI0 外设引脚:
      - SCK  (PB5) : SPI时钟
      - PICO (PB6) : SPI主机输出 (MOSI)
      - POCI (PB7) : SPI主机输入 (MISO, ST7735不需要)

    控制引脚 (GPIO):
      - CS  (PB0) : 片选
      - DC  (PB1) : 数据/命令选择
      - RST (PB4) : 复位
      - BLK (PB11): 背光

    可根据实际PCB连线修改以下宏定义
========================================================*/

/*---------- SPI0 外设引脚 ----------*/
#define LCD_SPI_INST                    SPI0

#define LCD_SCK_PORT                    GPIOB
#define LCD_SCK_PIN                     DL_GPIO_PIN_5
#define LCD_SCK_IOMUX                   (IOMUX_PINCM7)
#define LCD_SCK_IOMUX_FUNC              IOMUX_PINCM7_PF_SPI0_SCK

#define LCD_MOSI_PORT                   GPIOB
#define LCD_MOSI_PIN                    DL_GPIO_PIN_6
#define LCD_MOSI_IOMUX                  (IOMUX_PINCM8)
#define LCD_MOSI_IOMUX_FUNC             IOMUX_PINCM8_PF_SPI0_PICO

/*---------- GPIO 控制引脚 ----------*/
#define LCD_CS_PORT                     GPIOB
#define LCD_CS_PIN                      DL_GPIO_PIN_0
#define LCD_CS_IOMUX                    (IOMUX_PINCM3)

#define LCD_DC_PORT                     GPIOB
#define LCD_DC_PIN                      DL_GPIO_PIN_1
#define LCD_DC_IOMUX                    (IOMUX_PINCM4)

#define LCD_RST_PORT                    GPIOB
#define LCD_RST_PIN                     DL_GPIO_PIN_4
#define LCD_RST_IOMUX                   (IOMUX_PINCM6)

#define LCD_BLK_PORT                    GPIOB
#define LCD_BLK_PIN                     DL_GPIO_PIN_11
#define LCD_BLK_IOMUX                   (IOMUX_PINCM27)

/*========================================================
    ST7735 命令定义
========================================================*/
#define ST7735_NOP          0x00    /* 空操作                */
#define ST7735_SWRESET      0x01    /* 软件复位              */
#define ST7735_RDDID        0x04    /* 读显示ID              */
#define ST7735_RDDST        0x09    /* 读显示状态            */
#define ST7735_SLPIN        0x10    /* 睡眠模式              */
#define ST7735_SLPOUT       0x11    /* 退出睡眠              */
#define ST7735_PTLON        0x12    /* 局部显示模式          */
#define ST7735_NORON        0x13    /* 正常显示模式          */
#define ST7735_INVOFF       0x20    /* 反转显示关闭          */
#define ST7735_INVON        0x21    /* 反转显示开启          */
#define ST7735_DISPOFF      0x28    /* 显示关闭              */
#define ST7735_DISPON       0x29    /* 显示开启              */
#define ST7735_CASET        0x2A    /* 列地址设置            */
#define ST7735_RASET        0x2B    /* 行地址设置            */
#define ST7735_RAMWR        0x2C    /* 内存写入              */
#define ST7735_RAMRD        0x2E    /* 内存读取              */
#define ST7735_PTLAR        0x30    /* 局部显示区域          */
#define ST7735_COLMOD       0x3A    /* 像素格式设置          */
#define ST7735_MADCTL       0x36    /* 内存数据访问控制      */
#define ST7735_FRMCTR1      0xB1    /* 帧速率控制(正常模式)  */
#define ST7735_FRMCTR2      0xB2    /* 帧速率控制(空闲模式)  */
#define ST7735_FRMCTR3      0xB3    /* 帧速率控制(局部模式)  */
#define ST7735_INVCTR       0xB4    /* 反转控制              */
#define ST7735_DISSET5      0xB6    /* 显示设置5             */
#define ST7735_PWCTR1       0xC0    /* 电源控制1             */
#define ST7735_PWCTR2       0xC1    /* 电源控制2             */
#define ST7735_PWCTR3       0xC2    /* 电源控制3(正常模式)   */
#define ST7735_PWCTR4       0xC3    /* 电源控制4(空闲模式)   */
#define ST7735_PWCTR5       0xC4    /* 电源控制5(局部模式)   */
#define ST7735_VMCTR1       0xC5    /* VCOM控制1             */
#define ST7735_VMOFCTR      0xC7    /* VCOM偏移控制          */
#define ST7735_GMCTRP1      0xE0    /* Gamma校正 (+极性)     */
#define ST7735_GMCTRN1      0xE1    /* Gamma校正 (-极性)     */
#define ST7735_PWCTR6       0xFC    /* 电源控制6(局部模式)   */

/*========================================================
    MADCTL 寄存器位定义 (内存数据访问控制)
========================================================*/
#define ST7735_MADCTL_MY    0x80    /* 行地址顺序: 自底向上  */
#define ST7735_MADCTL_MX    0x40    /* 列地址顺序: 自右向左  */
#define ST7735_MADCTL_MV    0x20    /* 行/列交换             */
#define ST7735_MADCTL_ML    0x10    /* 垂直刷新顺序          */
#define ST7735_MADCTL_BGR   0x08    /* RGB/BGR顺序           */
#define ST7735_MADCTL_MH    0x04    /* 水平刷新顺序          */

/*========================================================
    颜色模式定义
========================================================*/
#define ST7735_COLMOD_12BIT 0x03    /* 12位色 (RGB 4-4-4)   */
#define ST7735_COLMOD_16BIT 0x05    /* 16位色 (RGB 5-6-5)   */
#define ST7735_COLMOD_18BIT 0x06    /* 18位色 (RGB 6-6-6)   */

/*========================================================
    函数声明
========================================================*/
void LCD_GPIO_Init(void);
void LCD_Init(void);

#endif /* __LCD_INIT_H */
