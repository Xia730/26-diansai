/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_MSPM0G350X
#define CONFIG_MSPM0G3507

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)



#define CPUCLK_FREQ                                                     32000000




/* Defines for I2C_OLED */
#define I2C_OLED_INST                                                       I2C0
#define I2C_OLED_INST_IRQHandler                                 I2C0_IRQHandler
#define I2C_OLED_INST_INT_IRQN                                     I2C0_INT_IRQn
#define I2C_OLED_BUS_SPEED_HZ                                             400000
#define GPIO_I2C_OLED_SDA_PORT                                             GPIOA
#define GPIO_I2C_OLED_SDA_PIN                                      DL_GPIO_PIN_0
#define GPIO_I2C_OLED_IOMUX_SDA                                   (IOMUX_PINCM1)
#define GPIO_I2C_OLED_IOMUX_SDA_FUNC                    IOMUX_PINCM1_PF_I2C0_SDA
#define GPIO_I2C_OLED_SCL_PORT                                             GPIOA
#define GPIO_I2C_OLED_SCL_PIN                                      DL_GPIO_PIN_1
#define GPIO_I2C_OLED_IOMUX_SCL                                   (IOMUX_PINCM2)
#define GPIO_I2C_OLED_IOMUX_SCL_FUNC                    IOMUX_PINCM2_PF_I2C0_SCL


/* Defines for UART0 */
#define UART0_INST                                                         UART0
#define UART0_INST_FREQUENCY                                             4000000
#define UART0_INST_IRQHandler                                   UART0_IRQHandler
#define UART0_INST_INT_IRQN                                       UART0_INT_IRQn
#define GPIO_UART0_RX_PORT                                                 GPIOA
#define GPIO_UART0_TX_PORT                                                 GPIOA
#define GPIO_UART0_RX_PIN                                         DL_GPIO_PIN_11
#define GPIO_UART0_TX_PIN                                         DL_GPIO_PIN_10
#define GPIO_UART0_IOMUX_RX                                      (IOMUX_PINCM22)
#define GPIO_UART0_IOMUX_TX                                      (IOMUX_PINCM21)
#define GPIO_UART0_IOMUX_RX_FUNC                       IOMUX_PINCM22_PF_UART0_RX
#define GPIO_UART0_IOMUX_TX_FUNC                       IOMUX_PINCM21_PF_UART0_TX
#define UART0_BAUD_RATE                                                   (9600)
#define UART0_IBRD_4_MHZ_9600_BAUD                                          (26)
#define UART0_FBRD_4_MHZ_9600_BAUD                                           (3)
/* Defines for UART3 */
#define UART3_INST                                                         UART3
#define UART3_INST_FREQUENCY                                             4000000
#define UART3_INST_IRQHandler                                   UART3_IRQHandler
#define UART3_INST_INT_IRQN                                       UART3_INT_IRQn
#define GPIO_UART3_RX_PORT                                                 GPIOB
#define GPIO_UART3_TX_PORT                                                 GPIOB
#define GPIO_UART3_RX_PIN                                          DL_GPIO_PIN_3
#define GPIO_UART3_TX_PIN                                          DL_GPIO_PIN_2
#define GPIO_UART3_IOMUX_RX                                      (IOMUX_PINCM16)
#define GPIO_UART3_IOMUX_TX                                      (IOMUX_PINCM15)
#define GPIO_UART3_IOMUX_RX_FUNC                       IOMUX_PINCM16_PF_UART3_RX
#define GPIO_UART3_IOMUX_TX_FUNC                       IOMUX_PINCM15_PF_UART3_TX
#define UART3_BAUD_RATE                                                 (115200)
#define UART3_IBRD_4_MHZ_115200_BAUD                                         (2)
#define UART3_FBRD_4_MHZ_115200_BAUD                                        (11)




/* Defines for SPI_LCD */
#define SPI_LCD_INST                                                       SPI1
#define SPI_LCD_INST_IRQHandler                                 SPI1_IRQHandler
#define SPI_LCD_INST_INT_IRQN                                     SPI1_INT_IRQn
#define GPIO_SPI_LCD_PICO_PORT                                            GPIOB
#define GPIO_SPI_LCD_PICO_PIN                                     DL_GPIO_PIN_8
#define GPIO_SPI_LCD_IOMUX_PICO                                 (IOMUX_PINCM25)
#define GPIO_SPI_LCD_IOMUX_PICO_FUNC                 IOMUX_PINCM25_PF_SPI1_PICO
#define GPIO_SPI_LCD_POCI_PORT                                            GPIOA
#define GPIO_SPI_LCD_POCI_PIN                                    DL_GPIO_PIN_16
#define GPIO_SPI_LCD_IOMUX_POCI                                 (IOMUX_PINCM38)
#define GPIO_SPI_LCD_IOMUX_POCI_FUNC                 IOMUX_PINCM38_PF_SPI1_POCI
/* GPIO configuration for SPI_LCD */
#define GPIO_SPI_LCD_SCLK_PORT                                            GPIOB
#define GPIO_SPI_LCD_SCLK_PIN                                     DL_GPIO_PIN_9
#define GPIO_SPI_LCD_IOMUX_SCLK                                 (IOMUX_PINCM26)
#define GPIO_SPI_LCD_IOMUX_SCLK_FUNC                 IOMUX_PINCM26_PF_SPI1_SCLK
#define GPIO_SPI_LCD_CS0_PORT                                             GPIOB
#define GPIO_SPI_LCD_CS0_PIN                                     DL_GPIO_PIN_20
#define GPIO_SPI_LCD_IOMUX_CS0                                  (IOMUX_PINCM48)
#define GPIO_SPI_LCD_IOMUX_CS0_FUNC                   IOMUX_PINCM48_PF_SPI1_CS0
/* Defines for SPI_IMU660RB */
#define SPI_IMU660RB_INST                                                  SPI0
#define SPI_IMU660RB_INST_IRQHandler                            SPI0_IRQHandler
#define SPI_IMU660RB_INST_INT_IRQN                                SPI0_INT_IRQn
#define GPIO_SPI_IMU660RB_PICO_PORT                                       GPIOB
#define GPIO_SPI_IMU660RB_PICO_PIN                               DL_GPIO_PIN_17
#define GPIO_SPI_IMU660RB_IOMUX_PICO                            (IOMUX_PINCM43)
#define GPIO_SPI_IMU660RB_IOMUX_PICO_FUNC            IOMUX_PINCM43_PF_SPI0_PICO
#define GPIO_SPI_IMU660RB_POCI_PORT                                       GPIOA
#define GPIO_SPI_IMU660RB_POCI_PIN                               DL_GPIO_PIN_13
#define GPIO_SPI_IMU660RB_IOMUX_POCI                            (IOMUX_PINCM35)
#define GPIO_SPI_IMU660RB_IOMUX_POCI_FUNC            IOMUX_PINCM35_PF_SPI0_POCI
/* GPIO configuration for SPI_IMU660RB */
#define GPIO_SPI_IMU660RB_SCLK_PORT                                       GPIOA
#define GPIO_SPI_IMU660RB_SCLK_PIN                               DL_GPIO_PIN_12
#define GPIO_SPI_IMU660RB_IOMUX_SCLK                            (IOMUX_PINCM34)
#define GPIO_SPI_IMU660RB_IOMUX_SCLK_FUNC            IOMUX_PINCM34_PF_SPI0_SCLK



/* Port definition for Pin Group GPIO_KEY */
#define GPIO_KEY_PORT                                                    (GPIOA)

/* Defines for PIN_KEY1: GPIOA.18 with pinCMx 40 on package pin 11 */
#define GPIO_KEY_PIN_KEY1_PIN                                   (DL_GPIO_PIN_18)
#define GPIO_KEY_PIN_KEY1_IOMUX                                  (IOMUX_PINCM40)
/* Port definition for Pin Group LED1 */
#define LED1_PORT                                                        (GPIOA)

/* Defines for RED: GPIOA.26 with pinCMx 59 on package pin 30 */
#define LED1_RED_PIN                                            (DL_GPIO_PIN_26)
#define LED1_RED_IOMUX                                           (IOMUX_PINCM59)
/* Defines for GREEN: GPIOA.24 with pinCMx 54 on package pin 25 */
#define LED1_GREEN_PIN                                          (DL_GPIO_PIN_24)
#define LED1_GREEN_IOMUX                                         (IOMUX_PINCM54)
/* Port definition for Pin Group SPI_LCD1 */
#define SPI_LCD1_PORT                                                    (GPIOB)

/* Defines for RES: GPIOB.10 with pinCMx 27 on package pin 62 */
#define SPI_LCD1_RES_PIN                                        (DL_GPIO_PIN_10)
#define SPI_LCD1_RES_IOMUX                                       (IOMUX_PINCM27)
/* Defines for DC: GPIOB.11 with pinCMx 28 on package pin 63 */
#define SPI_LCD1_DC_PIN                                         (DL_GPIO_PIN_11)
#define SPI_LCD1_DC_IOMUX                                        (IOMUX_PINCM28)
/* Defines for CS: GPIOB.14 with pinCMx 31 on package pin 2 */
#define SPI_LCD1_CS_PIN                                         (DL_GPIO_PIN_14)
#define SPI_LCD1_CS_IOMUX                                        (IOMUX_PINCM31)
/* Defines for BLK: GPIOB.26 with pinCMx 57 on package pin 28 */
#define SPI_LCD1_BLK_PIN                                        (DL_GPIO_PIN_26)
#define SPI_LCD1_BLK_IOMUX                                       (IOMUX_PINCM57)
/* Defines for PIN_IMU660RB_CS: GPIOA.2 with pinCMx 7 on package pin 42 */
#define GPIO_IMU660RB_PIN_IMU660RB_CS_PORT                               (GPIOA)
#define GPIO_IMU660RB_PIN_IMU660RB_CS_PIN                        (DL_GPIO_PIN_2)
#define GPIO_IMU660RB_PIN_IMU660RB_CS_IOMUX                       (IOMUX_PINCM7)
/* Defines for PIN_IMU660RB_INT1: GPIOB.13 with pinCMx 30 on package pin 1 */
#define GPIO_IMU660RB_PIN_IMU660RB_INT1_PORT                             (GPIOB)
// pins affected by this interrupt request:["PIN_IMU660RB_INT1"]
#define GPIO_IMU660RB_INT_IRQN                                  (GPIOB_INT_IRQn)
#define GPIO_IMU660RB_INT_IIDX                  (DL_INTERRUPT_GROUP1_IIDX_GPIOB)
#define GPIO_IMU660RB_PIN_IMU660RB_INT1_IIDX                (DL_GPIO_IIDX_DIO13)
#define GPIO_IMU660RB_PIN_IMU660RB_INT1_PIN                     (DL_GPIO_PIN_13)
#define GPIO_IMU660RB_PIN_IMU660RB_INT1_IOMUX                    (IOMUX_PINCM30)

/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);
void SYSCFG_DL_I2C_OLED_init(void);
void SYSCFG_DL_UART0_init(void);
void SYSCFG_DL_UART3_init(void);
void SYSCFG_DL_SPI_LCD_init(void);
void SYSCFG_DL_SPI_IMU660RB_init(void);


bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
