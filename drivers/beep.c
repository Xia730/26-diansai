#include "beep.h"
#include "ti_msp_dl_config.h"  // 改成你工程实际包含的头文件

/* ================== GPIO 宏 ================== */
#define BUZZER_ON()  DL_GPIO_setPins(GPIO_BEEP_PORT, GPIO_BEEP_PIN_1_PIN)
#define BUZZER_OFF() DL_GPIO_clearPins(GPIO_BEEP_PORT, GPIO_BEEP_PIN_1_PIN)

/* ================== 内部状态 ================== */
static uint8_t buzzer_flag = 0;     // 0:关  1:响
static uint16_t buzzer_counter = 0; // 计数器，单位：10ms

/* ================== 公共函数 ================== */

/**
 * @brief  调用此函数启动蜂鸣器响 1 秒
 */
void Buzzer_Beep(uint32_t t)
{
    BUZZER_ON();
    buzzer_flag = 1;
    buzzer_counter = t/10; // 10ms * 100 = 1000ms = 1秒
}

/**
 * @brief  定时器 10ms 中断里调用此函数，非阻塞控制蜂鸣器
 */
void Buzzer_Timer10msHandler(void)
{
    if(buzzer_flag)
    {
        if(buzzer_counter > 0)
        {
            buzzer_counter--;
        }
        else
        {
            BUZZER_OFF();
            buzzer_flag = 0;
        }
    }
}