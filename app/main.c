#include "main.h"




/* ================================================
 *  系统心跳
 * ================================================ */
volatile uint32_t sys_tick = 0;
volatile uint8_t g_10ms_flag = 0;

/* ================================================
 *  main — 程序入口
 *
 *  ── 状态机架构：4页面 ──
 *
 *    SELECT ─K3→ RUNNING ─K3→ PAUSED
 *      ↑         ↓ K4            ↓ K4
 *      └─────────┴────────────────┘ → SELECT
 *      ↓ K4(最后一项)
 *    PARAM → K4 → SELECT
 *
 *  ── 按键约定（全局统一）──
 *    K1 : 上移 / 加
 *    K2 : 下移 / 减
 *    K3 : 确认 / 进入 / 暂停
 *    K4 : 返回 / 停止
 *
 *  ── 主循环节奏（双轨）──
 *    高速控制轨：每次 while 都调 task->run()
 *    低速界面轨：每10ms（TIMER0产生g_10ms_flag）
 *                → KEY_Scan → MENU_KeyHandler → MENU_Refresh
 *
 *  ── 写新任务的步骤 ──
 *    1. tasks.c 里写 taskN_init / run / stop
 *    2. tasks.c 底部的 tasks[] 注册表加一行
 *    3. 参数在 params.h/c 加（可选）
 *    4. 自定义 LCD 显示写 draw() 注册进去（可选）
 *
 *  ── 底层的生产者-消费者 ──
 *    TIMER0中断每1ms→sys_tick++
 *    每10ms→g_10ms_flag = 1（主循环消费）
 *    UART0中断→bin_array[12], bin_ready（task_run消费）
 * ================================================ */
int main(void)
{
    SYSCFG_DL_init();
    LCD_Init();
    PARAM_Init();
    MENU_Init();
		IMU660RB_Init();
		
    NVIC_EnableIRQ(UART0_INST_INT_IRQN);
    NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);
	NVIC_EnableIRQ(TIMER_1_INST_INT_IRQN);
    DL_Timer_startCounter(TIMER_0_INST);

    Interrupt_Init();
	DL_TimerG_startCounter(PWM_A_INST);
	DL_TimerG_startCounter(PWM_B_INST);
	DL_TimerG_startCounter(TIMER_1_INST);
	Motor_Control(1, 0);
	Motor_Control(2, 0);

    Zigbee_Init();
    Maxicam_Init();
	
	//等待初始化稳定
		delay_ms(100);
    while (1) {
			
        /* ── 高速控制：每次循环都跑 ──
         * task->run() 不阻塞，几微秒就返回
         * 任务内部用 sys_tick 或 bin_ready 自己控制节奏
         */
        if (g_active_task && g_task_run) {
            g_active_task->run(bin_array, sys_tick);
        }

        /* ── 低速界面：每10ms一次 ──
         * 按键扫描 + LCD刷新不需要高频率
         */
        if (g_10ms_flag) {
            g_10ms_flag = 0;

            KEY_Scan();
            if (key) {
                MENU_KeyHandler(key);
                key = 0;
            }

            MENU_Refresh();
       }
    }
}

/* ================================================
 *  TIMER0 中断 — 1ms 系统心跳
 * ================================================ */
void TIMER_0_INST_IRQHandler(void)
{
    switch (DL_TimerG_getPendingInterrupt(TIMER_0_INST)) {
        case DL_TIMER_IIDX_ZERO:
            sys_tick++;
            
            if (sys_tick % 10 == 0) g_10ms_flag = 1;
				//if (sys_tick % 10 == 0) my_printf(UART1,"%0.1f\n",speed_l);;
            break;
        default:
            break;
    }
}

void TIMER_1_INST_IRQHandler(void)
{
    switch (DL_TimerG_getPendingInterrupt(TIMER_1_INST)) {
        case DL_TIMER_IIDX_ZERO:
        {
            ans_ten_us++;
            speed_check_cnt++;
						//yaw = Angle_180_to_360(euler.angle.yaw);
						
            if(speed_check_cnt >= SPEED_TIMEOUT)
            {
                speed_check_cnt = 0;

                /* ===== 左轮超时判断 ===== */
                if((ans_ten_us - ans_last_l) > SPEED_TIMEOUT)
                {
                    speed_l = 0;
                }
                /* ===== 右轮超时判断 ===== */
                if((ans_ten_us - ans_last_r) > SPEED_TIMEOUT)
                {
                    speed_r = 0;
                }
            }
        }
        default:
            break;
    }
}

/*********************** 函数声明 ************************/

/**
 * @brief 角度转换函数
 * @param angle 输入角度（-180° ~ 180°）
 * @return 转换后角度（0° ~ 360°）
 */
float Angle_180_to_360(float angle)
{
    while(angle < 0)
        angle += 360;

    while(angle >= 360)
        angle -= 360;

    return angle;
}