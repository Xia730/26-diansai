#ifndef __ZIGBEE_H
#define __ZIGBEE_H

#include "ti_msp_dl_config.h"

/* UART3 帧协议 */
#define Zigbee_START   '!'   /* 帧头 */
#define Zigbee_END    '@'   /* 帧尾 */
#define Zigbee_FRAME_MAX  64  /* 一帧最大字节数 */

extern volatile uint8_t  Zigbee_ready;  /* 1 = 收到完整一帧 */
extern uint8_t           Zigbee_frame_len;    /* 本帧数据长度 */
extern uint8_t           Zigbee_frame_buf[Zigbee_FRAME_MAX]; /* 帧数据 */

void Zigbee_Init(void);
void Zigbee_Send(const uint8_t *data);


#endif