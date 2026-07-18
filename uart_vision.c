#include "uart_vision.h"
#include "board.h"
#include <stdint.h>
#include <stdbool.h>

/* 若 board.h 中未定义 __IO，则在此处定义 */
#ifndef __IO
#define __IO volatile
#endif

/* 延时宏，使用 CPU 周期延时 */
#define delay_ms(X)    delay_cycles((CPUCLK_FREQ/1000)*(X))

/* ---------- 环形队列相关定义 ---------- */
#define FIFO_SIZE   128          // 队列深度，可根据需要调整

typedef struct {
    uint16_t buffer[FIFO_SIZE];
    volatile uint16_t ptrRead;   // volatile 保证中断可见
    volatile uint16_t ptrWrite;
} FIFO_t;

/* 全局队列实例 */
__IO FIFO_t rxFIFO = {0};

/* ---------- 帧接收标志及缓冲区 ---------- */
__IO bool rxFrameFlag = false;
__IO uint8_t rxCmd[FIFO_SIZE] = {0};
__IO uint8_t rxCount = 0;

/* ---------- 队列操作函数 ---------- */

/**
  * @brief  初始化队列
  * @param  无
  * @retval 无
  */
void initQueue(void)
{
    rxFIFO.ptrRead  = 0;
    rxFIFO.ptrWrite = 0;
}

/**
  * @brief  入队
  * @param  data 待入队的数据
  * @retval 无
  */
void fifo_enQueue(uint16_t data)
{
    rxFIFO.buffer[rxFIFO.ptrWrite] = data;
    ++rxFIFO.ptrWrite;
    if (rxFIFO.ptrWrite >= FIFO_SIZE)
    {
        rxFIFO.ptrWrite = 0;
    }
}

/**
  * @brief  出队
  * @param  无
  * @retval 出队的数据
  */
uint16_t fifo_deQueue(void)
{
    uint16_t element = 0;
    element = rxFIFO.buffer[rxFIFO.ptrRead];
    ++rxFIFO.ptrRead;
    if (rxFIFO.ptrRead >= FIFO_SIZE)
    {
        rxFIFO.ptrRead = 0;
    }
    return element;
}

/**
  * @brief  判断队列是否为空
  * @param  无
  * @retval true 为空，false 非空
  */
bool fifo_isEmpty(void)
{
    return (rxFIFO.ptrRead == rxFIFO.ptrWrite);
}

/**
  * @brief  获取队列长度
  * @param  无
  * @retval 当前队列中的数据个数
  */
uint16_t fifo_queueLength(void)
{
    if (rxFIFO.ptrRead <= rxFIFO.ptrWrite)
    {
        return (rxFIFO.ptrWrite - rxFIFO.ptrRead);
    }
    else
    {
        return (FIFO_SIZE - rxFIFO.ptrRead + rxFIFO.ptrWrite);
    }
}

/* ---------- 发送函数（基于 board.h 的 Uart_SendChar） ---------- */

/**
  * @brief   USART发送一个字节（直接寄存器操作，带超时）
  * @param   data : 待发送的数据
  * @retval  无
  */
void usart_SendByte(uint16_t data)
{
    __IO uint16_t t0 = 0;

    // 向发送数据寄存器写入一字节（MSPM0 UART 数据寄存器为 TXDATA）
    UART3_INST->TXDATA = (uint8_t)(data & 0xFF);

    // 等待发送 FIFO 空（STAT 寄存器的 TXFE 位为 1 表示空）
    while (!(UART3_INST->STAT & UART_STAT_TXFE_MASK))
    {
        if (++t0 > 8000)
        {
            return;   // 超时退出，防止死循环
        }
    }
}

/**
  * @brief   发送多个字节
  * @param   cmd : 待发送数据数组首地址
  * @param   len : 发送数据长度
  * @retval  无
  */
void usart_SendCmd(__IO uint8_t *cmd, uint8_t len)
{
    uint8_t i;
    for (i = 0; i < len; i++)
    {
        usart_SendByte(cmd[i]);
    }
}

/* ---------- UART3 中断服务函数 ---------- */
/**
  * @brief   USART3中断函数
  * @param   无
  * @retval  无
  */
void UART3_INST_IRQHandler(void)
{
    uint16_t i;

    switch (DL_UART_Main_getPendingInterrupt(UART3_INST))
    {
        case DL_UART_IIDX_RX:   // 接收中断
            // 读取一个字节并放入 FIFO
            fifo_enQueue(DL_UART_Main_receiveData(UART3_INST));
            break;

        case DL_UART_IIDX_RX_TIMEOUT_ERROR:// 空闲中断（需在 SysConfig 中使能）
        {
            // 清除空闲中断标志
            DL_UART_Main_clearInterruptStatus(UART3_INST, DL_UART_INTERRUPT_RX_TIMEOUT_ERROR);

            // 从 FIFO 中取出一帧数据
            rxCount = fifo_queueLength();
            for (i = 0; i < rxCount; i++)
            {
                rxCmd[i] = fifo_deQueue();
            }

            // 置位帧接收完成标志
            rxFrameFlag = true;
            break;
        }

        default:
            break;
    }
}