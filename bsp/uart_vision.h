#ifndef __uart_vision_H
#define __uart_vision_H

#include <stdint.h>
#include <stdarg.h>
#include "ti_msp_dl_config.h"
#include <string.h>
#include <stdio.h>


/* 指定串口发送字符 */
void Uart_SendChar(UART_Regs *uart, uint8_t data);

/* 指定串口发送字符串 */
void Uart_SendString(UART_Regs *uart, const char *str);

/* 指定串口发送数组 */
void Uart_SendArray(UART_Regs *uart, const uint8_t *buf, uint16_t length);

/* 使用可变参数实现的类 printf 函数 */
int my_printf(UART_Regs *uart,const char *format, ...);




#endif
