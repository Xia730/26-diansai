#ifndef __OLED_H
#define __OLED_H 

#include "board.h"
#include "stdlib.h"
#include "stdint.h"

#define OLED_CMD  0	// 写命令
#define OLED_DATA 1	// 写数据


/* OLED初始化 */
void oled_init(void);

/* 清屏 */
void oled_clear(void);

/* 更新显存 */
void oled_refresh(void);

/* 使用printf函数打印格式化字符串 */
void oled_printf(uint8_t Line, char *format, ...);

void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t size1,uint8_t mode);

#endif

