#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "board.h"




/*------------------------------------------------
作用    : 延时指定纳秒数
参数    : __us: 纳秒数
返回值  : 无
------------------------------------------------*/
void delay_us(uint32_t __us)
{
    delay_cycles((CPUCLK_FREQ / 1000000) * __us);
}

/*------------------------------------------------
作用    : 延时指定毫秒数
参数    : __ms: 毫秒数
返回值  : 无
------------------------------------------------*/
void delay_ms(uint32_t __ms)
{
    delay_cycles((CPUCLK_FREQ / 1000) * __ms);
}

/*------------------------------------------------
作用    : 指定串口发送字符
参数    : uart: 串口句柄
参数    : data: 待发送的字符
返回值  : 无
------------------------------------------------*/
void Uart_SendChar(UART_Regs *uart, uint8_t data)
{
    //等待串口空闲
    while( DL_UART_isBusy(uart) == true );

    //发送单个字符
    DL_UART_Main_transmitData(uart, data);

    // DL_UART_Main_transmitDataBlocking(uart, data);
}

/*------------------------------------------------
作用    : 指定串口发送字符串
参数    : uart: 串口句柄
参数    : str:  待发送的字符串
返回值  : 无
------------------------------------------------*/
void Uart_SendString(UART_Regs *uart, const char *str)
{
    //当前字符串地址不在结尾 并且 字符串首地址不为空
    while (str != 0 && *str != 0)
    {
        //发送字符串首地址中的字符，并且在发送完成之后首地址自增
        Uart_SendChar(uart, *str++);
    }
}

/*------------------------------------------------
作用    : 指定串口发送数组
参数    : uart: 串口句柄
参数    : buf:  待发送的数组 length:数组长度
返回值  : 无
------------------------------------------------*/
void Uart_SendArray(UART_Regs *uart, const uint8_t *buf, uint16_t length)
{
    if(buf == 0 |length == 0) return;
		uint16_t i;
		for(i = 0;i < length;i++)
	{
		Uart_SendChar(uart,buf[i]);
	}
		
    
}

/*------------------------------------------------
作用    : 使用可变参数是实现的类printf函数
参数    : format: 格式化字符串
参数    : ...:    可变参数
返回值  : 发送的字符串长度
------------------------------------------------*/
int my_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    // 创建一个足够大的缓冲区来存储格式化后的字符串
    char buffer[64] = {0};
    int len = vsnprintf(buffer, sizeof(buffer), format, args);

    va_end(args);

    // 发送格式化后的字符串
    Uart_SendString(printf_uart, buffer);

    return len;
}

/* 将c库的printf函数重新定位到USART */
int fputc(int ch, FILE *f)
{
    // 发送单个字符
    Uart_SendChar(printf_uart, (uint8_t)ch);
    return ch;
}

/* 重定向fputs函数 */
int fputs(const char* restrict s, FILE* restrict stream) {

    uint16_t char_len=0;
    while(*s!=0)
    {
        while( DL_UART_isBusy(printf_uart) == true );
        DL_UART_Main_transmitData(printf_uart, *s++);
        char_len++;
    }
    return char_len;
}

/* ================================================================
 * UART0 接收 (用于PC模拟视觉模块)
 * 电脑通过 USB转串口 → UART0 RX (PA11, 9600波特率)
 * 发送 ASCII 坐标字符串, 以换行符 '\n' 结尾
 * 例如: PC 发送 "123,89\n" → ISR 收完一行后置 rxFrameFlag0
 * ================================================================ */

/* UART0 接收缓冲区 (直接写入, 不用FIFO) */
__IO bool    rxFrameFlag0 = false;
__IO uint8_t rxCmd0[UART0_FIFO_SIZE] = {0};
__IO uint8_t rxCount0 = 0;

/* -------- UART0 中断服务函数 -------- */

/**
 * @brief  UART0 中断处理函数
 *         逐字节接收PC发来的模拟视觉数据
 *         检测到换行符 '\n' 时认为一帧结束, 置位 rxFrameFlag0
 *         也支持 '\r' 作为行尾 (兼容 Windows \r\n 和 Unix \n)
 */
void UART0_INST_IRQHandler(void)
{
    uint8_t ch;

    switch (DL_UART_Main_getPendingInterrupt(UART0_INST))
    {
        case DL_UART_IIDX_RX:
            /* 读取接收到的字节 */
            ch = DL_UART_Main_receiveData(UART0_INST);

            if (ch == '\r') {
                /* 忽略回车符 (Windows \r\n 格式) */
            } else if (ch == '\n') {
                /* 换行符 = 一帧结束 */
                rxCmd0[rxCount0] = '\0';    /* 字符串结尾 */
                rxFrameFlag0 = true;         /* 通知主循环 */
            } else if (rxCount0 < (UART0_FIFO_SIZE - 1)) {
                /* 普通字符: 存入缓冲区 */
                rxCmd0[rxCount0] = ch;
                rxCount0++;
            }
            /* 缓冲区满也视为帧结束 (防止溢出) */
            if (rxCount0 >= (UART0_FIFO_SIZE - 1)) {
                rxCmd0[rxCount0] = '\0';
                rxFrameFlag0 = true;
            }
            break;

        case DL_UART_IIDX_RX_TIMEOUT_ERROR:
            /* 超时: 如果缓冲区有数据未处理, 也视为一帧结束 */
            DL_UART_Main_clearInterruptStatus(UART0_INST,
                DL_UART_INTERRUPT_RX_TIMEOUT_ERROR);
            if (rxCount0 > 0 && !rxFrameFlag0) {
                rxCmd0[rxCount0] = '\0';
                rxFrameFlag0 = true;
            }
            break;

        default:
            break;
    }
}

/* -------- UART0 接收初始化 -------- */

/**
 * @brief  初始化 UART0 接收功能
 *         使能 NVIC 中断、配置 RX 超时作为兜底
 *         在 main() 中 SYSCFG_DL_init() 之后调用
 */
void UART0_RX_Init(void)
{
    /* 使能 UART0 的 NVIC 中断 */
    NVIC_EnableIRQ(UART0_INST_INT_IRQN);

    /* 设置 RX 超时: 30 bit周期 (9600下约3.1ms, 兜底用) */
    DL_UART_setRXInterruptTimeout(UART0_INST, 30);

    /* 使能 RX 超时中断 (RX中断已在 SYSCFG_DL_UART0_init 中使能) */
    DL_UART_Main_enableInterrupt(UART0_INST,
        DL_UART_MAIN_INTERRUPT_RX_TIMEOUT_ERROR);
}

