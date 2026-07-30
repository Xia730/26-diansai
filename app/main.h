#ifndef _MAIN_H
#define _MAIN_H

#include "board.h"
#include "lcd.h"
#include "key.h"
#include "trailing.h"
#include "menu.h"
#include "tasks.h"
#include "params.h"
#include "uart_vision.h"
#include "maxicam.h"
#include "motor/Motor.h"
#include "ti_msp_dl_config.h"
#include "bsp/interrupt.h"

/* ================================================
 *  按键变量
 * ================================================ */
uint8_t key = 0;

/*********************** 函数声明 ************************/

/**
 * @brief 角度转换函数
 * @param angle 输入角度（-180° ~ 180°）
 * @return 转换后角度（0° ~ 360°）
 */
float Angle_180_to_360(float angle);

/*********************** 陀螺仪相关 ************************/

/**
 * @brief 当前航向角（0~360°）
 */
float yaw = 0;


/*********************** 编码器相关 ************************/

/**
 * @brief 编码器无脉冲超时时间
 * @note  单位：10us（由定时器周期决定）
 *        超过该时间未检测到脉冲，则认为速度为0
 */
#define SPEED_TIMEOUT   20000   

/**
 * @brief 编码器时间基准计数（10us递增）
 */
uint32_t ans_ten_us = 0;

/**
 * @brief 无脉冲计数器（用于速度清零判断）
 */
uint32_t speed_check_cnt = 0;
volatile int32_t speed_l = 0;
volatile int32_t speed_r = 0;
volatile int32_t enc_count_l = 0;
volatile int32_t enc_count_r = 0;
#endif