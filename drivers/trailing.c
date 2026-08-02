#include "trailing.h"
#include "../app/params.h"
#include "lcd.h"

volatile uint8_t bin_ready=0 ;
volatile uint8_t buf_index=0 ;
volatile uint8_t receiving=0 ;

uint8_t bin_array[12];//最终数组
uint8_t ascii_buf[FRAME_LEN + 1];//缓冲数组

void UART_Process(uint8_t rx)//中断接收函数
{
    if (!receiving) {
        if (rx == START_BYTE) {
            receiving = 1;
            buf_index = 0;
        }
        return;
    }

    if (rx == END_BYTE) {

        if (1) {

  
            for (uint8_t i = 0; i < FRAME_LEN; i++) {
                if (ascii_buf[i] == '0')
                    bin_array[i] = 0;
                else if (ascii_buf[i] == '1')
                    bin_array[i] = 1;
                else {
                    receiving = 0;
                    return;
                }
            }
            bin_ready = 1; 
        }

        receiving = 0;  
        return;
    }
    if (rx == '0' || rx == '1') {
        if (buf_index < FRAME_LEN) {
            ascii_buf[buf_index++] = rx;
        } else {
      
            receiving = 0;
        }
    } 
    else {
        
        receiving = 0;
    }
}

// UART0 接收灰度数据中断
void UART0_INST_IRQHandler(void)
{
    if (DL_UART_Main_getPendingInterrupt(UART0_INST) == DL_UART_MAIN_IIDX_RX) {
        uint8_t rx = DL_UART_Main_receiveData(UART0_INST);
        UART_Process(rx);
			//DL_UART_Main_transmitData(UART0_INST, rx);  // 加这句回显
    }
}

void UART3_SendStates(volatile uint8_t bin_array[12])
{

    for (int i = 0; i<12; i++) {
        DL_UART_Main_transmitDataBlocking(UART3_INST, bin_array[i]+'0');  
    }
    DL_UART_Main_transmitDataBlocking(UART3_INST, '\n');
}

/* ================================================
 *  PD 循迹转向（加权和 + 丢线恢复）
 *
 *  每 10ms 跑一次
 *  输入：12路灰度传感器（0=白 1=黑）
 *  输出：差速值 correction（叠加到基础速度）
 *
 *  算法：
 *    Step1 — 加权和求位置
 *      weights[] = {-24,-18,-14,-10,-4,-2, 2,4,10,14,18,24}
 *      position = Σ(weight[i] × sensor[i]) / count
 *      负值=线偏左，正值=线偏右
 *
 *    Step2 — PD 控制
 *      correction = KP * position + KD * (position - last_position)
 *
 *    Step3 — 丢线恢复
 *      连续丢线 >3 次 → 根据 last_sum 方9向自旋寻线
 *
 *  ★ tuning：
 *    KP 调转向力度（太大=蛇形，太小=冲出）
 *    KD 抑制震荡（一般为 0）
 * ================================================ */
#define TRAIL_DT           10
#define MAX_CORRECTION     1000
#define LOST_MAX            3
#define LOST_SPIN            400

static const int8_t weight[12] = {-24,-18,-14,-11,-6,-2, 2,6,11,14,18,24};

int16_t Trail_Steering_Compute(const uint8_t sensor[12], uint32_t tick_ms)
{
    static uint32_t last_tick = 0;
    static int32_t  correction = 0;
    static float    last_position = 0.0f;
    static int32_t  last_sum = 0;
    static uint8_t  lost_cnt = 0;
    static uint8_t  ever_seen = 0;   // 是否收到过有效数据

    if (tick_ms - last_tick < TRAIL_DT) return (int16_t)correction;
    last_tick = tick_ms;

    /* ── Step1: 加权和求位置 ── */
    int32_t sum   = 0;
    uint8_t count = 0;

    for (uint8_t i = 0; i < 12; i++) {
        if (sensor[i]) {
            sum   += weight[i];
            count ++;
        }
    }

    /* ── 丢线处理 ── */
    if (count == 0) {
        if (!ever_seen) return 0;      // 还没见过线，不动作
        lost_cnt++;
        if (lost_cnt > LOST_MAX) {
            correction = (last_sum >= 0) ? -LOST_SPIN : LOST_SPIN;
        }
        return (int16_t)correction;
    }
    lost_cnt = 0;
    ever_seen = 1;
    last_sum = sum;

    /* ── Step2: PD 控制 ── */
    float position = (float)sum / (float)count;
    float delta    = position - last_position;
    float corr_f   = (param_trail_kp / 100.0f) * position
                    + (param_trail_kd / 100.0f) * delta;

    correction = -(int32_t)corr_f;

    /* ── 限幅 ── */
    if (correction >  MAX_CORRECTION) correction =  MAX_CORRECTION;
    if (correction < -MAX_CORRECTION) correction = -MAX_CORRECTION;

    last_position = position;

    return (int16_t)correction;
}

/* ── 启停线检测（≥4 路黑） ── */
uint8_t Trail_AllBlack(const uint8_t sensor[12])
{
    uint8_t cnt = 0;
    for (uint8_t i = 0; i < 12; i++) {
        if (sensor[i]) cnt++;
    }
    return (cnt >= 4) ? 1 : 0;
}

/* ── 停止线检测（带消抖） ── */
uint8_t Trail_DetectStopLine(const uint8_t sensor[12])
{
    static uint8_t armed = 1;   // 0=等待离开, 1=可以检测
    uint8_t cnt = 0;

    for (uint8_t i = 0; i < 12; i++) {
        if (sensor[i]) cnt++;
    }

    /* 先离开（<2 黑=正常线），准备好下次检测 */
    if (cnt < 2) {
        armed = 1;
        return 0;
    }

    /* ≥4 黑 + 已武装 → 触发 */
    if (cnt >= 4 && armed) {
        armed = 0;
        return 1;
    }

    return 0;
}


/* ================================================
 *  传感器黑线可视化 — LCD 上画 12 路状态条
 *
 *  每路传感器用一个小方块表示：
 *    填充黑色 = 检测到黑线 (sensor[i]=1)
 *    填充白色 = 白底       (sensor[i]=0)
 *    灰色边框 = 统一外框
 *
 *  布局：12 个方块居中排列，每块 16x16，间距 2px
 * ================================================ */
void Trail_DrawSensorBar(const uint8_t sensor[12], uint16_t y)
{
	uint8_t  i;
	uint16_t x, idx_pos;
	uint16_t block_w  = 16;
	uint16_t block_h  = 16;
	uint16_t gap      = 2;
	uint16_t start_x  = (LCD_W - (12 * block_w + 11 * gap)) / 2;  /* ~13 */
	uint16_t bar_top  = y + 22;   /* 留 22px 给文字行 */

	/* --- 标题行 --- */
	lcd_printf(0, y, BLACK, WHITE, "Sensors:");

	/* --- 索引号（1~12，标在方块上方）--- */
	for (i = 0; i < 12; i++) {
		x = start_x + i * (block_w + gap);
		idx_pos = x + 4;
		LCD_ShowChar(idx_pos, bar_top - 12, '1' + i, GRAY, WHITE, 16, 0);
	}

	/* --- 12 个方块 --- */
	for (i = 0; i < 12; i++) {
		x = start_x + i * (block_w + gap);

		/* 填充：黑线=黑色，白底=白色 */
		uint16_t fill = sensor[i] ? BLACK : WHITE;

		LCD_Fill(x, bar_top, x + block_w, bar_top + block_h, fill);
		LCD_DrawRectangle(x, bar_top, x + block_w, bar_top + block_h, GRAY);
	}
}
