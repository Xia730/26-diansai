#include "maxicam.h"

/* ========== UART1 帧接收 (中断) ==========
 *  协议:  ! <数据> @
 *  收到 '!' 开始攒数据, 收到 '@' 表示一帧结束
 *  数据自动累积到 Zigbee_frame_buf 中
 */

volatile uint8_t  Maxicam_ready = 0;
uint8_t           Maxicam_frame_len   = 0;
uint8_t           Maxicam_frame_buf[Maxicam_FRAME_MAX];

static uint8_t Maxicam_receiving = 0;   /* 帧内接收标志 */

void Maxicam_Send(const uint8_t *data)
{
    DL_UART_Main_transmitDataBlocking(UART1_INST, Maxicam_START);
    while (*data)
        DL_UART_Main_transmitDataBlocking(UART1_INST, *data++);
    DL_UART_Main_transmitDataBlocking(UART1_INST, Maxicam_END);
}

void Maxicam_Init(void)
{
    Maxicam_ready = 0;
    Maxicam_frame_len   = 0;
    Maxicam_receiving     = 0;
    NVIC_EnableIRQ(UART1_INST_INT_IRQN);
}

void UART1_IRQHandler(void)
{
    uint8_t ch;

    switch (DL_UART_Main_getPendingInterrupt(UART1_INST)) {
        case DL_UART_MAIN_IIDX_RX:
            while (!DL_UART_Main_isRXFIFOEmpty(UART1_INST)) {
                ch = DL_UART_Main_receiveData(UART1_INST);

                if (!Maxicam_receiving) {
                    if (ch == Maxicam_START) {          /* 检测帧头 */
                        Maxicam_receiving = 1;
                        Maxicam_frame_len = 0;
                    }
                    continue;
                }

                if (ch == Maxicam_END) {              /* 检测帧尾 */
                    Maxicam_ready = 1;
                    Maxicam_receiving = 0;
                    continue;
                }

                if (Maxicam_frame_len < Maxicam_FRAME_MAX) {
                    Maxicam_frame_buf[Maxicam_frame_len++] = ch;
                }
            }
            break;

        default:
            break;
    }
}