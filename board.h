#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>
#include <stdarg.h>
#include "ti_msp_dl_config.h"

/* 使用串口 */
#define printf_uart UART0

/* 延时函数 */
void delay_us(uint32_t us);
void delay_ms(uint32_t ms);

/* 指定串口发送字符 */
void Uart_SendChar(UART_Regs *uart, uint8_t data);

/* 指定串口发送字符串 */
void Uart_SendString(UART_Regs *uart, const char *str);

/* 指定串口发送数组 */
void Uart_SendArray(UART_Regs *uart, const uint8_t *buf, uint16_t length);

/* 使用可变参数实现的类 printf 函数 */
int my_printf(const char *format, ...);

void HR_SendBPM(uint8_t value);

/* ================================================================
 * UART0 接收 (用于PC模拟视觉模块发送坐标数据)
 * 电脑通过USB转串口连接 UART0 (PA10/TX, PA11/RX, 9600波特率)
 * 发送 ASCII 坐标字符串, 格式如 "123,456\n"
 * ================================================================ */
#define UART0_FIFO_SIZE     128     /* UART0 接收 FIFO 大小 */

extern __IO bool     rxFrameFlag0;  /* UART0 帧接收完成标志 */
extern __IO uint8_t  rxCmd0[UART0_FIFO_SIZE]; /* UART0 接收缓冲区 */
extern __IO uint8_t  rxCount0;      /* UART0 接收字节数 */

void UART0_RX_Init(void);           /* 初始化 UART0 接收 */

#endif