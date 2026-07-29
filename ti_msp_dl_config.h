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


#define GPIO_HFXT_PORT                                                     GPIOA
#define GPIO_HFXIN_PIN                                             DL_GPIO_PIN_5
#define GPIO_HFXIN_IOMUX                                         (IOMUX_PINCM10)
#define GPIO_HFXOUT_PIN                                            DL_GPIO_PIN_6
#define GPIO_HFXOUT_IOMUX                                        (IOMUX_PINCM11)
#define CPUCLK_FREQ                                                     80000000



/* Defines for PWM_A */
#define PWM_A_INST                                                         TIMG7
#define PWM_A_INST_IRQHandler                                   TIMG7_IRQHandler
#define PWM_A_INST_INT_IRQN                                     (TIMG7_INT_IRQn)
#define PWM_A_INST_CLK_FREQ                                             40000000
/* GPIO defines for channel 0 */
#define GPIO_PWM_A_C0_PORT                                                 GPIOA
#define GPIO_PWM_A_C0_PIN                                         DL_GPIO_PIN_26
#define GPIO_PWM_A_C0_IOMUX                                      (IOMUX_PINCM59)
#define GPIO_PWM_A_C0_IOMUX_FUNC                     IOMUX_PINCM59_PF_TIMG7_CCP0
#define GPIO_PWM_A_C0_IDX                                    DL_TIMER_CC_0_INDEX
/* GPIO defines for channel 1 */
#define GPIO_PWM_A_C1_PORT                                                 GPIOA
#define GPIO_PWM_A_C1_PIN                                         DL_GPIO_PIN_24
#define GPIO_PWM_A_C1_IOMUX                                      (IOMUX_PINCM54)
#define GPIO_PWM_A_C1_IOMUX_FUNC                     IOMUX_PINCM54_PF_TIMG7_CCP1
#define GPIO_PWM_A_C1_IDX                                    DL_TIMER_CC_1_INDEX

/* Defines for PWM_B */
#define PWM_B_INST                                                         TIMA0
#define PWM_B_INST_IRQHandler                                   TIMA0_IRQHandler
#define PWM_B_INST_INT_IRQN                                     (TIMA0_INT_IRQn)
#define PWM_B_INST_CLK_FREQ                                             40000000
/* GPIO defines for channel 2 */
#define GPIO_PWM_B_C2_PORT                                                 GPIOB
#define GPIO_PWM_B_C2_PIN                                         DL_GPIO_PIN_12
#define GPIO_PWM_B_C2_IOMUX                                      (IOMUX_PINCM29)
#define GPIO_PWM_B_C2_IOMUX_FUNC                     IOMUX_PINCM29_PF_TIMA0_CCP2
#define GPIO_PWM_B_C2_IDX                                    DL_TIMER_CC_2_INDEX
/* GPIO defines for channel 3 */
#define GPIO_PWM_B_C3_PORT                                                 GPIOB
#define GPIO_PWM_B_C3_PIN                                         DL_GPIO_PIN_13
#define GPIO_PWM_B_C3_IOMUX                                      (IOMUX_PINCM30)
#define GPIO_PWM_B_C3_IOMUX_FUNC                     IOMUX_PINCM30_PF_TIMA0_CCP3
#define GPIO_PWM_B_C3_IDX                                    DL_TIMER_CC_3_INDEX

/* Defines for PWM_1 */
#define PWM_1_INST                                                         TIMA1
#define PWM_1_INST_IRQHandler                                   TIMA1_IRQHandler
#define PWM_1_INST_INT_IRQN                                     (TIMA1_INT_IRQn)
#define PWM_1_INST_CLK_FREQ                                             80000000
/* GPIO defines for channel 0 */
#define GPIO_PWM_1_C0_PORT                                                 GPIOA
#define GPIO_PWM_1_C0_PIN                                         DL_GPIO_PIN_10
#define GPIO_PWM_1_C0_IOMUX                                      (IOMUX_PINCM21)
#define GPIO_PWM_1_C0_IOMUX_FUNC                     IOMUX_PINCM21_PF_TIMA1_CCP0
#define GPIO_PWM_1_C0_IDX                                    DL_TIMER_CC_0_INDEX
/* GPIO defines for channel 1 */
#define GPIO_PWM_1_C1_PORT                                                 GPIOA
#define GPIO_PWM_1_C1_PIN                                         DL_GPIO_PIN_11
#define GPIO_PWM_1_C1_IOMUX                                      (IOMUX_PINCM22)
#define GPIO_PWM_1_C1_IOMUX_FUNC                     IOMUX_PINCM22_PF_TIMA1_CCP1
#define GPIO_PWM_1_C1_IDX                                    DL_TIMER_CC_1_INDEX



/* Defines for TIMER_0 */
#define TIMER_0_INST                                                     (TIMG0)
#define TIMER_0_INST_IRQHandler                                 TIMG0_IRQHandler
#define TIMER_0_INST_INT_IRQN                                   (TIMG0_INT_IRQn)
#define TIMER_0_INST_LOAD_VALUE                                          (4999U)
/* Defines for TIMER_1 */
#define TIMER_1_INST                                                    (TIMG12)
#define TIMER_1_INST_IRQHandler                                TIMG12_IRQHandler
#define TIMER_1_INST_INT_IRQN                                  (TIMG12_INT_IRQn)
#define TIMER_1_INST_LOAD_VALUE                                           (799U)




/* Defines for I2C_OLED */
#define I2C_OLED_INST                                                       I2C0
#define I2C_OLED_INST_IRQHandler                                 I2C0_IRQHandler
#define I2C_OLED_INST_INT_IRQN                                     I2C0_INT_IRQn
#define I2C_OLED_BUS_SPEED_HZ                                             400000
#define GPIO_I2C_OLED_SDA_PORT                                             GPIOA
#define GPIO_I2C_OLED_SDA_PIN                                     DL_GPIO_PIN_28
#define GPIO_I2C_OLED_IOMUX_SDA                                   (IOMUX_PINCM3)
#define GPIO_I2C_OLED_IOMUX_SDA_FUNC                    IOMUX_PINCM3_PF_I2C0_SDA
#define GPIO_I2C_OLED_SCL_PORT                                             GPIOA
#define GPIO_I2C_OLED_SCL_PIN                                     DL_GPIO_PIN_31
#define GPIO_I2C_OLED_IOMUX_SCL                                   (IOMUX_PINCM6)
#define GPIO_I2C_OLED_IOMUX_SCL_FUNC                    IOMUX_PINCM6_PF_I2C0_SCL


/* Defines for UART0 */
#define UART0_INST                                                         UART0
#define UART0_INST_FREQUENCY                                            40000000
#define UART0_INST_IRQHandler                                   UART0_IRQHandler
#define UART0_INST_INT_IRQN                                       UART0_INT_IRQn
#define GPIO_UART0_RX_PORT                                                 GPIOA
#define GPIO_UART0_TX_PORT                                                 GPIOA
#define GPIO_UART0_RX_PIN                                          DL_GPIO_PIN_1
#define GPIO_UART0_TX_PIN                                          DL_GPIO_PIN_0
#define GPIO_UART0_IOMUX_RX                                       (IOMUX_PINCM2)
#define GPIO_UART0_IOMUX_TX                                       (IOMUX_PINCM1)
#define GPIO_UART0_IOMUX_RX_FUNC                        IOMUX_PINCM2_PF_UART0_RX
#define GPIO_UART0_IOMUX_TX_FUNC                        IOMUX_PINCM1_PF_UART0_TX
#define UART0_BAUD_RATE                                                 (115200)
#define UART0_IBRD_40_MHZ_115200_BAUD                                       (21)
#define UART0_FBRD_40_MHZ_115200_BAUD                                       (45)
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
/* Defines for UART1 */
#define UART1_INST                                                         UART1
#define UART1_INST_FREQUENCY                                            40000000
#define UART1_INST_IRQHandler                                   UART1_IRQHandler
#define UART1_INST_INT_IRQN                                       UART1_INT_IRQn
#define GPIO_UART1_RX_PORT                                                 GPIOA
#define GPIO_UART1_TX_PORT                                                 GPIOA
#define GPIO_UART1_RX_PIN                                          DL_GPIO_PIN_9
#define GPIO_UART1_TX_PIN                                          DL_GPIO_PIN_8
#define GPIO_UART1_IOMUX_RX                                      (IOMUX_PINCM20)
#define GPIO_UART1_IOMUX_TX                                      (IOMUX_PINCM19)
#define GPIO_UART1_IOMUX_RX_FUNC                       IOMUX_PINCM20_PF_UART1_RX
#define GPIO_UART1_IOMUX_TX_FUNC                       IOMUX_PINCM19_PF_UART1_TX
#define UART1_BAUD_RATE                                                 (115200)
#define UART1_IBRD_40_MHZ_115200_BAUD                                       (21)
#define UART1_FBRD_40_MHZ_115200_BAUD                                       (45)
/* Defines for UART2 */
#define UART2_INST                                                         UART2
#define UART2_INST_FREQUENCY                                            40000000
#define UART2_INST_IRQHandler                                   UART2_IRQHandler
#define UART2_INST_INT_IRQN                                       UART2_INT_IRQn
#define GPIO_UART2_RX_PORT                                                 GPIOB
#define GPIO_UART2_TX_PORT                                                 GPIOB
#define GPIO_UART2_RX_PIN                                         DL_GPIO_PIN_16
#define GPIO_UART2_TX_PIN                                         DL_GPIO_PIN_15
#define GPIO_UART2_IOMUX_RX                                      (IOMUX_PINCM33)
#define GPIO_UART2_IOMUX_TX                                      (IOMUX_PINCM32)
#define GPIO_UART2_IOMUX_RX_FUNC                       IOMUX_PINCM33_PF_UART2_RX
#define GPIO_UART2_IOMUX_TX_FUNC                       IOMUX_PINCM32_PF_UART2_TX
#define UART2_BAUD_RATE                                                 (115200)
#define UART2_IBRD_40_MHZ_115200_BAUD                                       (21)
#define UART2_FBRD_40_MHZ_115200_BAUD                                       (45)




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
/* Defines for SPI_IMU660RB */
#define SPI_IMU660RB_INST                                                  SPI0
#define SPI_IMU660RB_INST_IRQHandler                            SPI0_IRQHandler
#define SPI_IMU660RB_INST_INT_IRQN                                SPI0_INT_IRQn
#define GPIO_SPI_IMU660RB_PICO_PORT                                       GPIOB
#define GPIO_SPI_IMU660RB_PICO_PIN                               DL_GPIO_PIN_17
#define GPIO_SPI_IMU660RB_IOMUX_PICO                            (IOMUX_PINCM43)
#define GPIO_SPI_IMU660RB_IOMUX_PICO_FUNC            IOMUX_PINCM43_PF_SPI0_PICO
#define GPIO_SPI_IMU660RB_POCI_PORT                                       GPIOB
#define GPIO_SPI_IMU660RB_POCI_PIN                               DL_GPIO_PIN_19
#define GPIO_SPI_IMU660RB_IOMUX_POCI                            (IOMUX_PINCM45)
#define GPIO_SPI_IMU660RB_IOMUX_POCI_FUNC            IOMUX_PINCM45_PF_SPI0_POCI
/* GPIO configuration for SPI_IMU660RB */
#define GPIO_SPI_IMU660RB_SCLK_PORT                                       GPIOB
#define GPIO_SPI_IMU660RB_SCLK_PIN                               DL_GPIO_PIN_18
#define GPIO_SPI_IMU660RB_IOMUX_SCLK                            (IOMUX_PINCM44)
#define GPIO_SPI_IMU660RB_IOMUX_SCLK_FUNC            IOMUX_PINCM44_PF_SPI0_SCLK



/* Defines for DMA_LCD_TX */
#define DMA_LCD_TX_CHAN_ID                                                   (0)
#define SPI_LCD_INST_DMA_TRIGGER                              (DMA_SPI1_TX_TRIG)



/* Port definition for Pin Group GPIO_BEEP */
#define GPIO_BEEP_PORT                                                   (GPIOB)

/* Defines for PIN_1: GPIOB.25 with pinCMx 56 on package pin 27 */
#define GPIO_BEEP_PIN_1_PIN                                     (DL_GPIO_PIN_25)
#define GPIO_BEEP_PIN_1_IOMUX                                    (IOMUX_PINCM56)
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
/* Port definition for Pin Group GPIO_IMU660RB */
#define GPIO_IMU660RB_PORT                                               (GPIOA)

/* Defines for PIN_IMU660RB_CS: GPIOA.2 with pinCMx 7 on package pin 42 */
#define GPIO_IMU660RB_PIN_IMU660RB_CS_PIN                        (DL_GPIO_PIN_2)
#define GPIO_IMU660RB_PIN_IMU660RB_CS_IOMUX                       (IOMUX_PINCM7)
/* Defines for PIN_IMU660RB_INT1: GPIOA.21 with pinCMx 46 on package pin 17 */
// pins affected by this interrupt request:["PIN_IMU660RB_INT1"]
#define GPIO_IMU660RB_INT_IRQN                                  (GPIOA_INT_IRQn)
#define GPIO_IMU660RB_INT_IIDX                  (DL_INTERRUPT_GROUP1_IIDX_GPIOA)
#define GPIO_IMU660RB_PIN_IMU660RB_INT1_IIDX                (DL_GPIO_IIDX_DIO21)
#define GPIO_IMU660RB_PIN_IMU660RB_INT1_PIN                     (DL_GPIO_PIN_21)
#define GPIO_IMU660RB_PIN_IMU660RB_INT1_IOMUX                    (IOMUX_PINCM46)
/* Defines for PIN_KEY1: GPIOA.18 with pinCMx 40 on package pin 11 */
#define GPIO_KEY_PIN_KEY1_PORT                                           (GPIOA)
#define GPIO_KEY_PIN_KEY1_PIN                                   (DL_GPIO_PIN_18)
#define GPIO_KEY_PIN_KEY1_IOMUX                                  (IOMUX_PINCM40)
/* Defines for PIN_KEY2: GPIOA.7 with pinCMx 14 on package pin 49 */
#define GPIO_KEY_PIN_KEY2_PORT                                           (GPIOA)
#define GPIO_KEY_PIN_KEY2_PIN                                    (DL_GPIO_PIN_7)
#define GPIO_KEY_PIN_KEY2_IOMUX                                  (IOMUX_PINCM14)
/* Defines for PIN_KEY3: GPIOB.1 with pinCMx 13 on package pin 48 */
#define GPIO_KEY_PIN_KEY3_PORT                                           (GPIOB)
#define GPIO_KEY_PIN_KEY3_PIN                                    (DL_GPIO_PIN_1)
#define GPIO_KEY_PIN_KEY3_IOMUX                                  (IOMUX_PINCM13)
/* Defines for PIN_KEY4: GPIOB.0 with pinCMx 12 on package pin 47 */
#define GPIO_KEY_PIN_KEY4_PORT                                           (GPIOB)
#define GPIO_KEY_PIN_KEY4_PIN                                    (DL_GPIO_PIN_0)
#define GPIO_KEY_PIN_KEY4_IOMUX                                  (IOMUX_PINCM12)
/* Defines for PIN_B: GPIOA.22 with pinCMx 47 on package pin 18 */
#define GPIO_ENCODER1_PIN_B_PORT                                         (GPIOA)
#define GPIO_ENCODER1_PIN_B_PIN                                 (DL_GPIO_PIN_22)
#define GPIO_ENCODER1_PIN_B_IOMUX                                (IOMUX_PINCM47)
/* Defines for PIN_A: GPIOB.24 with pinCMx 52 on package pin 23 */
#define GPIO_ENCODER1_PIN_A_PORT                                         (GPIOB)
// groups represented: ["GPIO_ENCODER2","GPIO_ENCODER1"]
// pins affected: ["PINA","PIN_A"]
#define GPIO_MULTIPLE_GPIOB_INT_IRQN                            (GPIOB_INT_IRQn)
#define GPIO_MULTIPLE_GPIOB_INT_IIDX            (DL_INTERRUPT_GROUP1_IIDX_GPIOB)
#define GPIO_ENCODER1_PIN_A_IIDX                            (DL_GPIO_IIDX_DIO24)
#define GPIO_ENCODER1_PIN_A_PIN                                 (DL_GPIO_PIN_24)
#define GPIO_ENCODER1_PIN_A_IOMUX                                (IOMUX_PINCM52)
/* Port definition for Pin Group GPIO_ENCODER2 */
#define GPIO_ENCODER2_PORT                                               (GPIOB)

/* Defines for PINA: GPIOB.4 with pinCMx 17 on package pin 52 */
#define GPIO_ENCODER2_PINA_IIDX                              (DL_GPIO_IIDX_DIO4)
#define GPIO_ENCODER2_PINA_PIN                                   (DL_GPIO_PIN_4)
#define GPIO_ENCODER2_PINA_IOMUX                                 (IOMUX_PINCM17)
/* Defines for PINB: GPIOB.5 with pinCMx 18 on package pin 53 */
#define GPIO_ENCODER2_PINB_PIN                                   (DL_GPIO_PIN_5)
#define GPIO_ENCODER2_PINB_IOMUX                                 (IOMUX_PINCM18)
/* Port definition for Pin Group GPIO_LED */
#define GPIO_LED_PORT                                                    (GPIOA)

/* Defines for Red: GPIOA.15 with pinCMx 37 on package pin 8 */
#define GPIO_LED_Red_PIN                                        (DL_GPIO_PIN_15)
#define GPIO_LED_Red_IOMUX                                       (IOMUX_PINCM37)
/* Defines for Green: GPIOA.17 with pinCMx 39 on package pin 10 */
#define GPIO_LED_Green_PIN                                      (DL_GPIO_PIN_17)
#define GPIO_LED_Green_IOMUX                                     (IOMUX_PINCM39)

/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);
void SYSCFG_DL_PWM_A_init(void);
void SYSCFG_DL_PWM_B_init(void);
void SYSCFG_DL_PWM_1_init(void);
void SYSCFG_DL_TIMER_0_init(void);
void SYSCFG_DL_TIMER_1_init(void);
void SYSCFG_DL_I2C_OLED_init(void);
void SYSCFG_DL_UART0_init(void);
void SYSCFG_DL_UART3_init(void);
void SYSCFG_DL_UART1_init(void);
void SYSCFG_DL_UART2_init(void);
void SYSCFG_DL_SPI_LCD_init(void);
void SYSCFG_DL_SPI_IMU660RB_init(void);
void SYSCFG_DL_DMA_init(void);


bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
