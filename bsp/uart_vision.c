#include "uart_vision.h"
#include "board.h"
#include <stdint.h>
#include <stdbool.h>


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
int my_printf(UART_Regs *uart,const char *format, ...)
{
    va_list args;
    va_start(args, format);

    // 创建一个足够大的缓冲区来存储格式化后的字符串
    char buffer[64] = {0};
    int len = vsnprintf(buffer, sizeof(buffer), format, args);

    va_end(args);

    // 发送格式化后的字符串
    Uart_SendString(uart, buffer);

    return len;
}

