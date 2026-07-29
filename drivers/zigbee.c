#include "zigbee.h"

/* ========== UART3 帧接收 (中断) ==========
 *  协议:  ! <数据> @
 *  收到 '!' 开始攒数据, 收到 '@' 表示一帧结束
 *  数据自动累积到 Zigbee_frame_buf 中
 */

volatile uint8_t  Zigbee_ready = 0;
uint8_t           Zigbee_frame_len   = 0;
uint8_t           Zigbee_frame_buf[Zigbee_FRAME_MAX];

static uint8_t Zigbee_receiving = 0;   /* 帧内接收标志 */

void Zigbee_Send(const uint8_t *data)
{
    DL_UART_Main_transmitDataBlocking(UART3_INST, Zigbee_START);
    while (*data)
        DL_UART_Main_transmitDataBlocking(UART3_INST, *data++);
    DL_UART_Main_transmitDataBlocking(UART3_INST, Zigbee_END);
}

void Zigbee_Init(void)
{
    Zigbee_ready = 0;
    Zigbee_frame_len   = 0;
    Zigbee_receiving     = 0;
    NVIC_EnableIRQ(UART3_INST_INT_IRQN);
}

void UART3_IRQHandler(void)
{
    uint8_t ch;

    switch (DL_UART_Main_getPendingInterrupt(UART3_INST)) {
        case DL_UART_MAIN_IIDX_RX:
            while (!DL_UART_Main_isRXFIFOEmpty(UART3_INST)) {
                ch = DL_UART_Main_receiveData(UART3_INST);

                if (!Zigbee_receiving) {
                    if (ch == Zigbee_START) {          /* 检测帧头 */
                        Zigbee_receiving = 1;
                        Zigbee_frame_len = 0;
                    }
                    continue;
                }

                if (ch == Zigbee_END) {              /* 检测帧尾 */
                    Zigbee_ready = 1;
                    Zigbee_receiving = 0;
                    continue;
                }

                if (Zigbee_frame_len < Zigbee_FRAME_MAX) {
                    Zigbee_frame_buf[Zigbee_frame_len++] = ch;
                }
            }
            break;

        default:
            break;
    }
}