#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#include "ti_msp_dl_config.h"
extern uint32_t ans_fist_l,ans_fist_r;
extern uint32_t ans_last_l,ans_last_r;
void Interrupt_Init(void);

#endif  /* #ifndef _INTERRUPT_H_ */