#ifndef __MAXICAM_H
#define __MAXICAM_H

#include "ti_msp_dl_config.h"

#define Maxicam_START     '@'   /* 帧头 */
#define Maxicam_END       '&'   /* 帧尾 */
#define Maxicam_FRAME_MAX  64

extern volatile uint8_t  Maxicam_ready;
extern uint8_t           Maxicam_frame_len;
extern uint8_t           Maxicam_frame_buf[Maxicam_FRAME_MAX];

void Maxicam_Init(void);
void Maxicam_Send(const uint8_t *data);

#endif