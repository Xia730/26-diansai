#ifndef __uart_vision_H
#define __uart_vision_H

#include <stdint.h>
#include "ti_msp_dl_config.h"
#include "uart_vision.h"

/*C语言头文件*/
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>



//volatile unsigned char uart_data = 0;

void uart0_send_char(char ch);
void uart0_send_string(char* str);
void uart0_send_num(uint8_t num);
void uart_rx_parse(uint8_t data);
//串口协议定义
#define UART_RX_BUF_LEN    32
#define FRAME_HEADER       0xAA
#define FRAME_TAIL         0x55
#define FRAME_LEN          9         // 一帧总长度：7字节

//与主机通信的变量数组
extern char USART0_RX_BUF[1024];
extern uint16_t USART0_RX_LEN;
extern uint8_t USART0_RX_FINISH;

//与esp8266通信的变量数组
extern char USART3_RX_BUF[1024];
extern uint16_t USART3_RX_LEN;
extern uint8_t USART3_RX_FINISH;

#endif
