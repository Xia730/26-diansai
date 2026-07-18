#ifndef __uart_vision_H
#define __uart_vision_H

#include <stdint.h>
#include "ti_msp_dl_config.h"

/*C���Կⶥ���ļ�*/
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define 	FIFO_SIZE   128

extern __IO bool rxFrameFlag;
extern __IO uint8_t rxCmd[FIFO_SIZE];
extern __IO uint8_t rxCount;


void fifo_initQueue(void);
void fifo_enQueue(uint16_t data);
uint16_t fifo_deQueue(void);
bool fifo_isEmpty(void);
uint16_t fifo_queueLength(void);

void usart_SendCmd(__IO uint8_t *cmd, uint8_t len);
void usart_SendByte(uint16_t data);

#endif
