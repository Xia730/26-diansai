#include "interrupt.h"
#include "board.h"
#include "imu660rb.h"
#include "interrupt.h"

extern volatile uint32_t ans_ten_us;
extern int32_t speed_l,speed_r;
extern volatile int32_t enc_count_l, enc_count_r;
int32_t speed_ans_l,speed_ans_r;
uint32_t ans_fist_l=0,ans_fist_r=0;
uint32_t ans_last_l=0,ans_last_r=0;

int Filter10_Sliding_l(int new_data)//左轮异常去除
{
    static int buf_l[10] = {0};
    static int index_l = 0;
    static int filled_l = 0;

    int i;
    int pos = 0, neg = 0;
    int sum = 0, count = 0;

    // 1. 写入滑动窗口
    buf_l[index_l++] = new_data;
    if (index_l >= 10) index_l = 0;
    if (filled_l < 10) filled_l++;

    // 不足10个直接返回
    if (filled_l < 10) return new_data;

    // 2. 判断主方向
    for (i = 0; i < 10; i++) {
        if (buf_l[i] >= 0) pos++;
        else neg++;
    }

    int main_sign = (pos >= neg) ? 1 : -1;

    // 3. 剔除反向异常
    for (i = 0; i < 10; i++) {
        if ((main_sign > 0 && buf_l[i] >= 0) ||
            (main_sign < 0 && buf_l[i] < 0)) {
            sum += buf_l[i];
            count++;
        }
    }

    if (count == 0) return 0;

    return sum / count;
}

int Filter10_Sliding_r(int new_data)//右轮异常去除
{
    static int buf_r[10] = {0};
    static int index_r = 0;
    static int filled_r = 0;

    int i;
    int pos = 0, neg = 0;
    int sum = 0, count = 0;

    // 1. 写入滑动窗口
    buf_r[index_r++] = new_data;
    if (index_r >= 10) index_r = 0;
    if (filled_r < 10) filled_r++;

    // 不足10个直接返回
    if (filled_r < 10) return new_data;

    // 2. 判断主方向
    for (i = 0; i < 10; i++) {
        if (buf_r[i] >= 0) pos++;
        else neg++;
    }

    int main_sign = (pos >= neg) ? 1 : -1;

    // 3. 剔除反向异常
    for (i = 0; i < 10; i++) {
        if ((main_sign > 0 && buf_r[i] >= 0) ||
            (main_sign < 0 && buf_r[i] < 0)) {
            sum += buf_r[i];
            count++;
        }
    }

    if (count == 0) return 0;

    return sum / count;
}


float Kalman_Filter_Simple_l(float z)//左轮测速卡尔曼
{
    static float x_l = 0.0f;   // 估计值
    static float P_l = 1.0f;   // 协方差
    static float Q_l = 0.01f;  // 过程噪声
    static float R_l = 2.0f;   // 测量噪声
    float K_l;

    // 预测
    P_l = P_l + Q_l;

    // 增益
    K_l = P_l / (P_l + R_l);

    // 更新
    x_l = x_l + K_l * (z - x_l);

    // 更新误差
    P_l = (1 - K_l) * P_l;

    return x_l;
}


float Kalman_Filter_Simple_r(float z)//右轮测速卡尔曼
{
    static float x_r = 0.0f;   // 估计值
    static float P_r = 1.0f;   // 协方差
    static float Q_r = 0.01f;  // 过程噪声
    static float R_r = 2.0f;   // 测量噪声
    float K_r;

    // 预测
    P_r = P_r + Q_r;

    // 增益
    K_r = P_r / (P_r + R_r);

    // 更新
    x_r = x_r + K_r * (z - x_r);

    // 更新误差
    P_r = (1 - K_r) * P_r;

    return x_r;
}

uint8_t enable_group1_irq = 1;

void Interrupt_Init(void)
{
    if(enable_group1_irq)
    {
        /* 清除所有 GPIO 挂起中断，防止上电瞬间的跳变触发 */
        DL_GPIO_clearInterruptStatus(GPIOB,
            GPIO_ENCODER1_PIN_A_PIN | GPIO_ENCODER2_PINA_PIN);
        DL_GPIO_clearInterruptStatus(GPIOA,
            GPIO_IMU660RB_PIN_IMU660RB_INT1_PIN);
        NVIC_EnableIRQ(1);
    }
}

void GROUP1_IRQHandler(void)
{
    switch (DL_Interrupt_getPendingGroup(DL_INTERRUPT_GROUP_1)) {

    /* ================= GPIOA — IMU660RB ================= */
//    #if defined GPIO_IMU660RB_INT_IIDX
//    case GPIO_IMU660RB_INT_IIDX:
//    {
//        uint32_t pending = DL_GPIO_getPendingInterrupt(GPIOA);

//        #if defined GPIO_IMU660RB_PIN_IMU660RB_INT1_IIDX
//        if (pending == GPIO_IMU660RB_PIN_IMU660RB_INT1_IIDX) {
//            Read_IMU660RB();
//            DL_GPIO_clearInterruptStatus(GPIO_IMU660RB_PORT,
//                GPIO_IMU660RB_PIN_IMU660RB_INT1_PIN);
//        }
//        #endif
//        break;
//    }
//    #endif

    /* ================= GPIOB — 编码器 ================= */
    #if defined GPIO_MULTIPLE_GPIOB_INT_IIDX
    case GPIO_MULTIPLE_GPIOB_INT_IIDX:
    {
        uint32_t pending = DL_GPIO_getPendingInterrupt(GPIOB);

        switch (pending)
        {
            /* ── 右轮编码器 Encoder2（A=GPIOB.4, B=GPIOB.5）── */
            #if defined GPIO_ENCODER2_PINA_IIDX
            case GPIO_ENCODER2_PINA_IIDX:
            {
                if(DL_GPIO_readPins(GPIOB, GPIO_ENCODER2_PINB_PIN))
                    enc_count_r--;
                else
                    enc_count_r++;
                ans_last_r = ans_ten_us;

                if(ans_last_r >= ans_fist_r)
                {
                    speed_ans_r = ans_last_r - ans_fist_r;

                    if(speed_ans_r>50 && speed_ans_r<10000)
                    {
                        if(DL_GPIO_readPins(GPIOB, GPIO_ENCODER2_PINB_PIN))
                            speed_r = -100000 / speed_ans_r;
                        else
                            speed_r = 100000 / speed_ans_r;
                        speed_r = Filter10_Sliding_r(speed_r);
                        speed_r = Kalman_Filter_Simple_r(speed_r);
                    }
                    else
                    {
                        ans_fist_r = ans_last_r;
                        break;
                    }
                }
                ans_fist_r = ans_last_r;
                DL_GPIO_clearInterruptStatus(GPIO_ENCODER2_PORT,
                    GPIO_ENCODER2_PINA_PIN);
                break;
            }
            #endif

            /* ── 左轮编码器 Encoder1（A=GPIOB.24, B=GPIOA.22）── */
            #if defined GPIO_ENCODER1_PIN_A_IIDX
            case GPIO_ENCODER1_PIN_A_IIDX:
            {
                if(DL_GPIO_readPins(GPIOA, GPIO_ENCODER1_PIN_B_PIN))
                    enc_count_l++;
                else
                    enc_count_l--;
                ans_last_l = ans_ten_us;

                if(ans_last_l >= ans_fist_l)
                {
                    speed_ans_l = ans_last_l - ans_fist_l;

                    if(speed_ans_l>50&&speed_ans_l<10000)
                    {
                        if(DL_GPIO_readPins(GPIOA, GPIO_ENCODER1_PIN_B_PIN))
                        {
                            speed_l=100000/speed_ans_l;
                        }
                        else
                        {
                            speed_l=-100000/speed_ans_l;
                        }
                        speed_l= Filter10_Sliding_l(speed_l);
                        speed_l= Kalman_Filter_Simple_l(speed_l);
                    }
                    else
                    {
                        ans_fist_l = ans_last_l;
                        break;
                    }
                }
                ans_fist_l = ans_last_l;
                DL_GPIO_clearInterruptStatus(GPIO_ENCODER1_PIN_A_PORT,
                    GPIO_ENCODER1_PIN_A_PIN);
                break;
            }
            #endif

            default:
                break;
        }
        break;
    }
    #endif


    default:
        break;
    }
}

/* ========== DMA 中断处理 ========== */
void DMA_IRQHandler(void)
{
    switch (DL_DMA_getPendingInterrupt(DMA)) {
        case DL_DMA_EVENT_IIDX_DMACH0:
            /* DMA通道0传输完成（当前使用同步轮询，此处作为安全网） */
            break;
        default:
            break;
    }
}