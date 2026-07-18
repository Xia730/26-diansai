#include <stdio.h>
#include "board.h"       /* 硬件初始化, 延时, printf, UART0接收 */
#include "lcd.h"         /* LCD 图形函数 */
#include "lcd_port.h"    /* 底层 SPI (通过 board.c 实现) */
#include "Emm_V5.h"      /* Emm_V5 电机驱动库 */
#include "uart_vision.h" /* UART3 视觉数据接收 (MaixCAM2) */
#include "tracking.h"    /* 云台跟踪模块 */

/**********************************************************
***  二维云台目标跟踪系统 — 主程序
***  视觉输入 (双源自动切换):
***    UART0: PC模拟视觉 (USB转串口, 9600波特率) → 电脑发送坐标
***    UART3: MaixCAM2 (K230, 115200波特率) → 真实视觉模块
***  电机驱动: Emm_V5.0 × 2 → UART3 TX
***  显示屏:   ST7735 128x160 SPI LCD
***  主控:     MSPM0G3507 @ 32MHz
**********************************************************/

/* 全局跟踪系统实例 */
static tracking_system_t g_tracker;

/* 系统毫秒 tick (由主循环延时近似累积) */
static volatile uint32_t g_sys_tick_ms = 0;

/* 当前数据来源 (0=UART3视觉模块, 1=UART0电脑模拟) */
static uint8_t g_vision_source = 0;

/**
 * @brief  获取系统毫秒 tick
 */
uint32_t tracking_get_tick_ms(void)
{
    return g_sys_tick_ms;
}

int main(void)
{
    /* ---------- 1. 硬件初始化 ---------- */
    SYSCFG_DL_init();                               /* 所有外设初始化 */

    /* UART3 视觉模块接收初始化 */
    NVIC_EnableIRQ(UART3_INST_INT_IRQN);            /* 使能UART3中断 */
    DL_UART_setRXInterruptTimeout(UART3_INST, 3);   /* UART3 RX超时=3个字符周期 */

    /* UART0 PC模拟接收初始化 */
    UART0_RX_Init();                                /* 使能UART0中断+超时 */

    lcd_init();                                      /* LCD初始化 */

    /* ---------- 2. 清屏并显示静态标题 ---------- */
    lcd_fill_color(BLACK);
    lcd_printf(0, 5, YELLOW, BLACK, "== Gimbal Tracker ==");
    lcd_printf(0, 20, WHITE, BLACK, "UART0:PC UART3:CAM");
    lcd_printf(0, 110, WHITE, BLACK, "Init...");

    /* ---------- 3. 初始化跟踪模块 ---------- */
    tracking_init(&g_tracker);

    /* ---------- 4. 使能两个电机 ---------- */
    delay_ms(2000);  /* 等待系统稳定 */

    lcd_printf(0, 110, WHITE, BLACK, "Enabling motors...");
    Emm_V5_En_Control(MOTOR_PAN_ADDR, true, false);   /* 使能平移电机 */
    delay_ms(50);
    Emm_V5_En_Control(MOTOR_TILT_ADDR, true, false);  /* 使能俯仰电机 */
    delay_ms(1000);

    lcd_printf(0, 110, WHITE, BLACK, "Waiting vision...");

    /* ---------- 5. 主跟踪循环 ---------- */
    while (1)
    {
        /* 累加系统 tick (主循环约每15ms执行一次) */
        g_sys_tick_ms += 15;

        bool frame_valid = false;

        /* ---- 检查视觉帧 ----
         * 优先 UART3 (真实 MaixCAM2), 其次 UART0 (PC模拟)
         * 两者都有数据时 UART3 优先
         * ---- */
        if (rxFrameFlag)   /* UART3: 真实视觉模块 (MaixCAM2) */
        {
            rxFrameFlag = false;
            frame_valid = tracking_parse_coords(&g_tracker, (const uint8_t *)rxCmd, rxCount);
            if (frame_valid) {
                g_vision_source = 0;  /* 来源: 视觉模块 */
            }
        }
        else if (rxFrameFlag0)  /* UART0: PC模拟视觉 */
        {
            rxFrameFlag0 = false;
            frame_valid = tracking_parse_coords(&g_tracker, (const uint8_t *)rxCmd0, rxCount0);
            rxCount0 = 0;   /* 重置缓冲区指针, 准备接收下一行 */
            if (frame_valid) {
                g_vision_source = 1;  /* 来源: PC模拟 */
            }
        }

        /* 更新跟踪状态机 */
        tracking_update_state(&g_tracker, frame_valid, g_sys_tick_ms);

        /* 如果已锁定或正在搜索, 且有有效坐标 → 执行PID+电机控制 */
        if (frame_valid &&
            (g_tracker.state == TRACK_STATE_LOCKED ||
             g_tracker.state == TRACK_STATE_SEARCHING))
        {
            /* PID迭代 */
            tracking_pid_pan(&g_tracker);
            tracking_pid_tilt(&g_tracker);

            /* 转换为方向+绝对值 */
            tracking_prepare_motor_commands(&g_tracker);

            /* 发送电机命令 */
            tracking_send_motor_commands(&g_tracker);
        }

        /* 刷新LCD显示 */
        tracking_lcd_update(&g_tracker);

        /* 显示数据来源 (UART0=PC模拟, UART3=CAM视觉) */
        if (g_vision_source == 0) {
            lcd_printf(0, 110, GREEN, BLACK, "SRC:CAM        ");
        } else {
            lcd_printf(0, 110, CYAN, BLACK, "SRC:PC          ");
        }

        /* 主循环延时 (控制更新速率, ~50-60Hz) */
        delay_ms(15);
    }
}
