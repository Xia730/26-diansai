#include "uart_vision.h"
#include "board.h"
#include "oled.h"
#define delay_ms(X)    delay_cycles((CPUCLK_FREQ/1000)*(X))

//与主机通信的变量数组
char USART0_RX_BUF[1024] = {0};
uint16_t USART0_RX_LEN = {0};
uint8_t USART0_RX_FINISH = 0;

//与esp8266通信的变量数组
char USART3_RX_BUF[1024] = {0};
uint16_t USART3_RX_LEN = {0};
uint8_t USART3_RX_FINISH = 0;


// 发送数字 0~255
void uart0_send_num(uint8_t num)
{
    char buf[4];
    buf[0] = (num / 100) + '0';
    buf[1] = ((num / 10) % 10) + '0';
    buf[2] = (num % 10) + '0';
    buf[3] = '\0';
    uart0_send_string(buf);
}

// 发送单个字符
void uart0_send_char(char ch)
{
    while(DL_UART_isBusy(UART_0_INST));
    DL_UART_Main_transmitData(UART_0_INST, ch);
}

// 发送字符串
void uart0_send_string(char* str)
{
    while(*str != 0)
    {
        uart0_send_char(*str++);
    }
}


// 串口中断
void UART_0_INST_IRQHandler(void)
{
		uint8_t temp;
    //如果产生了串口中断
    switch( DL_UART_Main_getPendingInterrupt(UART_0_INST) )
    {
        case DL_UART_IIDX_RX://如果是接收中断
            //接发送过来的数据保存在变量中
            temp = DL_UART_Main_receiveData(UART_0_INST);
						USART0_RX_BUF[USART0_RX_LEN++] = temp;
				
					  //缓冲区防溢出
            if(USART0_RX_LEN >= 1023) USART0_RX_LEN = 0;
						//检测到指令结束符\n(对应\r\n)，标志接收完成
						if(temp == '\n')
						{
							USART0_RX_FINISH = 1;
						}
				
            break;

        default://其他的串口中断
            break;
    }
		
}

void UART_3_INST_IRQHandler(void)
{
		uint8_t temp;
    //如果产生了串口中断
    switch( DL_UART_Main_getPendingInterrupt(UART_3_INST) )
    {
        case DL_UART_IIDX_RX://如果是接收中断
            //接发送过来的数据保存在变量中
            temp = DL_UART_Main_receiveData(UART_3_INST);
						USART3_RX_BUF[USART3_RX_LEN++] = temp;
//		if(strstr(USART1_RX_BUF,"OK")!=NULL)
//{
//	oled_printf(7,"OK");
//}
					
				
					  //缓冲区防溢出
            if(USART3_RX_LEN >= 1023) USART3_RX_LEN = 0;
						//检测到指令结束符\n(对应\r\n)，标志接收完成
						if(temp == '\n')
						{
							USART3_RX_FINISH = 1;
						}
				
            break;

        default://其他的串口中断
            break;
    }
		
}

