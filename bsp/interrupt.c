#include "ti_msp_dl_config.h"
#include "interrupt.h"
#include "board.h"
#include "imu660rb.h"


uint8_t enable_group1_irq = 0;

void Interrupt_Init(void)
{
    if(enable_group1_irq)
    {
        NVIC_EnableIRQ(1);
    }
}

void GROUP1_IRQHandler(void)
{
    switch (DL_Interrupt_getPendingGroup(DL_INTERRUPT_GROUP_1)) {

    /* ================= GPIOA ================= */
    

    /* ================= GPIOB ================= */
   

    /* ================= 其他单独中断 ================= */
    #if defined GPIO_IMU660RB_INT_IIDX
    case GPIO_IMU660RB_INT_IIDX:
        Read_IMU660RB();
        break;
    #endif

    default:
        break;
    }
}