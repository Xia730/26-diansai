#include "lcd_init.h"
#include "lcd.h"
#include "ti/driverlib/dl_spi.h"

/*========================================================
    内部函数声明
========================================================*/
static void LCD_SPI_Init(void);
static void LCD_WriteCmd(uint8_t cmd);
static void LCD_WriteData(uint8_t dat);
static void LCD_WriteData16(uint16_t dat);

/*========================================================
    名称: LCD_WriteCmd
    作用: 通过硬件SPI发送ST7735命令
    参数: cmd - 8位命令字节
========================================================*/
static void LCD_WriteCmd(uint8_t cmd)
{
    DL_GPIO_clearPins(LCD_DC_PORT, LCD_DC_PIN);     /* DC=0: 命令模式 */
    DL_GPIO_clearPins(LCD_CS_PORT, LCD_CS_PIN);     /* CS=0: 片选拉低 */
    DL_SPI_transmitDataBlocking8(LCD_SPI_INST, cmd);
    DL_GPIO_setPins(LCD_CS_PORT, LCD_CS_PIN);       /* CS=1: 释放片选 */
}

/*========================================================
    名称: LCD_WriteData
    作用: 通过硬件SPI发送ST7735数据 (单字节)
    参数: dat - 8位数据字节
========================================================*/
static void LCD_WriteData(uint8_t dat)
{
    DL_GPIO_setPins(LCD_DC_PORT, LCD_DC_PIN);       /* DC=1: 数据模式 */
    DL_GPIO_clearPins(LCD_CS_PORT, LCD_CS_PIN);     /* CS=0: 片选拉低 */
    DL_SPI_transmitDataBlocking8(LCD_SPI_INST, dat);
    DL_GPIO_setPins(LCD_CS_PORT, LCD_CS_PIN);       /* CS=1: 释放片选 */
}

/*========================================================
    名称: LCD_WriteData16
    作用: 通过硬件SPI发送ST7735数据 (16位, RGB565颜色值)
    参数: dat - 16位数据
========================================================*/
static void LCD_WriteData16(uint16_t dat)
{
    DL_GPIO_setPins(LCD_DC_PORT, LCD_DC_PIN);       /* DC=1: 数据模式 */
    DL_GPIO_clearPins(LCD_CS_PORT, LCD_CS_PIN);     /* CS=0: 片选拉低 */
    DL_SPI_transmitDataBlocking8(LCD_SPI_INST, (uint8_t)(dat >> 8));
    DL_SPI_transmitDataBlocking8(LCD_SPI_INST, (uint8_t)(dat));
    DL_GPIO_setPins(LCD_CS_PORT, LCD_CS_PIN);       /* CS=1: 释放片选 */
}

/*========================================================
    名称: LCD_SPI_Init
    作用: 初始化SPI0外设 (硬件SPI主模式)
========================================================*/
static void LCD_SPI_Init(void)
{
    DL_SPI_ClockConfig spiClockCfg;
    DL_SPI_Config      spiCfg;

    /*--- 1. 开启SPI0电源并复位 ---*/
    DL_SPI_enablePower(LCD_SPI_INST);
    DL_SPI_reset(LCD_SPI_INST);

    /*--- 2. 配置SPI时钟 ---*/
    /* SPI时钟源 = BUSCLK (32MHz), 分频比 = 2, 即 SCLK = 16MHz */
    spiClockCfg.clockSel    = DL_SPI_CLOCK_BUSCLK;
    spiClockCfg.divideRatio = DL_SPI_CLOCK_DIVIDE_RATIO_2;
    DL_SPI_setClockConfig(LCD_SPI_INST, &spiClockCfg);

    /*--- 3. 配置SPI参数 ---*/
    /*
     * ST7735 要求:
     *   - SPI Mode 0: CPOL=0, CPHA=0
     *   - MSB First
     *   - 8-bit 数据帧
     *   - 主模式 (Controller)
     *   - 片选由GPIO手动控制
     */
    spiCfg.mode         = DL_SPI_MODE_CONTROLLER;
    spiCfg.frameFormat  = DL_SPI_FRAME_FORMAT_MOTO4_POL0_PHA0;
    spiCfg.dataSize     = DL_SPI_DATA_SIZE_8;
    spiCfg.bitOrder     = DL_SPI_BIT_ORDER_MSB_FIRST;
    spiCfg.chipSelectPin = DL_SPI_CHIP_SELECT_NONE;      /* CS由GPIO控制 */
    spiCfg.parity       = DL_SPI_PARITY_NONE;
    DL_SPI_init(LCD_SPI_INST, &spiCfg);

    /*--- 4. 使能SPI ---*/
    DL_SPI_enable(LCD_SPI_INST);
}

/*========================================================
    名称: LCD_GPIO_Init
    作用: 初始化LCD相关IO口
    说明: SPI引脚(SCK/MOSI)使用外设功能,
          CS/DC/RST/BLK 使用GPIO输出模式
========================================================*/
void LCD_GPIO_Init(void)
{
    /*--- 1. SPI功能引脚 (SCK + MOSI) ---*/
    DL_GPIO_initPeripheralOutputFunction(
        LCD_SCK_IOMUX, LCD_SCK_IOMUX_FUNC);
    DL_GPIO_initPeripheralOutputFunction(
        LCD_MOSI_IOMUX, LCD_MOSI_IOMUX_FUNC);

    /*--- 2. GPIO控制引脚 ---*/
    /* CS (片选) - 初始输出高电平(不选中) */
    DL_GPIO_initDigitalOutputFeatures(
        LCD_CS_IOMUX, DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_PULL_UP, DL_GPIO_DRIVE_STRENGTH_LOW,
        DL_GPIO_HIZ_DISABLE);
    DL_GPIO_setPins(LCD_CS_PORT, LCD_CS_PIN);
    DL_GPIO_enableOutput(LCD_CS_PORT, LCD_CS_PIN);

    /* DC (命令/数据) - 初始输出高电平 */
    DL_GPIO_initDigitalOutputFeatures(
        LCD_DC_IOMUX, DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_NONE, DL_GPIO_DRIVE_STRENGTH_LOW,
        DL_GPIO_HIZ_DISABLE);
    DL_GPIO_setPins(LCD_DC_PORT, LCD_DC_PIN);
    DL_GPIO_enableOutput(LCD_DC_PORT, LCD_DC_PIN);

    /* RST (复位) - 初始输出高电平(不复位) */
    DL_GPIO_initDigitalOutputFeatures(
        LCD_RST_IOMUX, DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_PULL_UP, DL_GPIO_DRIVE_STRENGTH_LOW,
        DL_GPIO_HIZ_DISABLE);
    DL_GPIO_setPins(LCD_RST_PORT, LCD_RST_PIN);
    DL_GPIO_enableOutput(LCD_RST_PORT, LCD_RST_PIN);

    /* BLK (背光) - 初始输出高电平(点亮) */
    DL_GPIO_initDigitalOutputFeatures(
        LCD_BLK_IOMUX, DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_NONE, DL_GPIO_DRIVE_STRENGTH_LOW,
        DL_GPIO_HIZ_DISABLE);
    DL_GPIO_setPins(LCD_BLK_PORT, LCD_BLK_PIN);
    DL_GPIO_enableOutput(LCD_BLK_PORT, LCD_BLK_PIN);

    /*--- 3. 初始化硬件SPI ---*/
    LCD_SPI_Init();
}

/*========================================================
    名称: LCD_Init
    作用: ST7735S 初始化序列
    说明: 上电后调用一次,完成LCD寄存器配置
========================================================*/
void LCD_Init(void)
{
    /*--- 1. 硬件复位 ---*/
    DL_GPIO_clearPins(LCD_RST_PORT, LCD_RST_PIN);
    delay_ms(10);
    DL_GPIO_setPins(LCD_RST_PORT, LCD_RST_PIN);
    delay_ms(120);

    /*--- 2. 软件复位 ---*/
    LCD_WriteCmd(ST7735_SWRESET);
    delay_ms(120);

    /*--- 3. 退出睡眠模式 ---*/
    LCD_WriteCmd(ST7735_SLPOUT);
    delay_ms(120);

    /*--- 4. 帧速率控制 ---*/
    LCD_WriteCmd(ST7735_FRMCTR1);
    LCD_WriteData(0x01);    /* 最快帧速率, DOTCLK/1   */
    LCD_WriteData(0x2C);    /* 行周期 44 clocks       */
    LCD_WriteData(0x2D);    /* 前沿 45 clocks         */

    LCD_WriteCmd(ST7735_FRMCTR2);
    LCD_WriteData(0x01);    /* DOTCLK/1               */
    LCD_WriteData(0x2C);
    LCD_WriteData(0x2D);

    LCD_WriteCmd(ST7735_FRMCTR3);
    LCD_WriteData(0x01);    /* DOTCLK/1               */
    LCD_WriteData(0x2C);
    LCD_WriteData(0x2D);
    LCD_WriteData(0x01);
    LCD_WriteData(0x2C);
    LCD_WriteData(0x2D);

    /*--- 5. 反转控制 ---*/
    LCD_WriteCmd(ST7735_INVCTR);
    LCD_WriteData(0x07);    /* 不反转                 */

    /*--- 6. 电源控制 ---*/
    LCD_WriteCmd(ST7735_PWCTR1);
    LCD_WriteData(0xA2);    /* AVDD = 5.0V            */
    LCD_WriteData(0x02);    /* AVCL = -5.0V           */
    LCD_WriteData(0x84);    /* VRHP                   */

    LCD_WriteCmd(ST7735_PWCTR2);
    LCD_WriteData(0xC5);    /* VGH = 15.0V            */

    LCD_WriteCmd(ST7735_PWCTR3);
    LCD_WriteData(0x0A);    /* 正常模式运算放大器电流 */
    LCD_WriteData(0x00);

    LCD_WriteCmd(ST7735_PWCTR4);
    LCD_WriteData(0x8A);    /* 空闲模式运算放大器电流 */
    LCD_WriteData(0x2A);

    LCD_WriteCmd(ST7735_PWCTR5);
    LCD_WriteData(0x8A);    /* 局部模式运算放大器电流 */
    LCD_WriteData(0xEE);

    /*--- 7. VCOM控制 ---*/
    LCD_WriteCmd(ST7735_VMCTR1);
    LCD_WriteData(0x0E);

    /*--- 8. Gamma校正 ---*/
    LCD_WriteCmd(ST7735_GMCTRP1);
    LCD_WriteData(0x02);
    LCD_WriteData(0x1C);
    LCD_WriteData(0x07);
    LCD_WriteData(0x12);
    LCD_WriteData(0x37);
    LCD_WriteData(0x32);
    LCD_WriteData(0x29);
    LCD_WriteData(0x2D);
    LCD_WriteData(0x29);
    LCD_WriteData(0x25);
    LCD_WriteData(0x2B);
    LCD_WriteData(0x39);
    LCD_WriteData(0x00);
    LCD_WriteData(0x01);
    LCD_WriteData(0x03);
    LCD_WriteData(0x10);

    LCD_WriteCmd(ST7735_GMCTRN1);
    LCD_WriteData(0x03);
    LCD_WriteData(0x1D);
    LCD_WriteData(0x07);
    LCD_WriteData(0x06);
    LCD_WriteData(0x2E);
    LCD_WriteData(0x2C);
    LCD_WriteData(0x29);
    LCD_WriteData(0x2D);
    LCD_WriteData(0x2E);
    LCD_WriteData(0x2E);
    LCD_WriteData(0x37);
    LCD_WriteData(0x3F);
    LCD_WriteData(0x00);
    LCD_WriteData(0x00);
    LCD_WriteData(0x02);
    LCD_WriteData(0x10);

    /*--- 9. 显示设置 ---*/
    LCD_WriteCmd(ST7735_COLMOD);
    LCD_WriteData(ST7735_COLMOD_16BIT);   /* 16位色 RGB 5-6-5 */

    /*--- 10. 内存访问控制 ---*/
    /* MY | MX | MV = 正常方向 (横屏或竖屏通过此寄存器调整) */
    LCD_WriteCmd(ST7735_MADCTL);
    LCD_WriteData(ST7735_MADCTL_MX | ST7735_MADCTL_MV | ST7735_MADCTL_BGR);
    /* 说明: MX=左右翻转, MV=行列交换, BGR=色彩顺序;
             竖屏用: MADCTL_MX | MADCTL_MY | MADCTL_BGR
             可根据需要修改此参数 */

    /*--- 11. 显示设置5 ---*/
    LCD_WriteCmd(ST7735_DISSET5);
    LCD_WriteData(0x15);    /* 1 = 使能灰色调控制    */
    LCD_WriteData(0x02);    /* 2 = BGR模式           */

    /*--- 12. 正常显示模式 ---*/
    LCD_WriteCmd(ST7735_NORON);
    delay_ms(10);

    /*--- 13. 开启显示 ---*/
    LCD_WriteCmd(ST7735_DISPON);
    delay_ms(10);

    /*--- 14. 背光点亮 ---*/
    DL_GPIO_setPins(LCD_BLK_PORT, LCD_BLK_PIN);
}
