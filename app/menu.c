#include "menu.h"
#include "params.h"
#include "tasks.h"
#include "trailing.h"
#include "lcd.h"
#include "motor/Motor.h"

extern int32_t speed_l, speed_r;
extern volatile int32_t enc_count_l, enc_count_r;
extern volatile uint32_t sys_tick;
extern volatile uint8_t g_paused;
extern uint32_t g_pause_accum, g_pause_start;
#include <stdio.h>
#include <string.h>

/* ================================================
 *  颜色定义（RGB565）
 * ================================================ */
#define BG_COLOR    WHITE
#define TXT_COLOR   BLACK
#define SEL_BG      BLUE
#define SEL_FG      WHITE
#define BAR_ON      BLUE
#define BAR_OFF     LGRAY
#define TITLE_COLOR DARKBLUE

/* ================================================
 *  LCD 屏幕坐标布局 (240×240)
 * ================================================ */
#define TITLE_Y     2
#define ITEM_Y0     30
#define ITEM_H      30
#define HINT_Y      216
#define BAR_Y       80
#define BAR_H       16

/* ================================================
 *  状态机核心变量
 * ================================================ */
static MenuPage page = MENU_SELECT;
static uint8_t cursor = 0;
static uint8_t param_cursor = 0;

static void draw_select(void);
static void draw_param(void);
static void draw_running(void);

void MENU_Init(void)
{
    page = MENU_SELECT;
    cursor = 0;
    param_cursor = 0;
    LCD_Fill(0, 0, LCD_W, LCD_H, BG_COLOR);
    draw_select();
}

void MENU_KeyHandler(uint8_t key)
{
    switch (page) {

    /* ──── 主菜单 ──── */
    case MENU_SELECT:
        if (key == 1 && cursor > 0) {
            cursor--;
            draw_select();
        } else if (key == 2 && cursor < TASK_COUNT) {
            cursor++;
            draw_select();
        } else if (key == 3) {
            if (cursor < TASK_COUNT) {
                TASK_Select(cursor);
                page = MENU_RUNNING;
                LCD_Fill(0, 0, LCD_W, LCD_H, BG_COLOR);
                draw_running();
            } else {
                param_cursor = 0;
                page = MENU_PARAM;
                LCD_Fill(0, 0, LCD_W, LCD_H, BG_COLOR);
                draw_param();
            }
        }
        break;

    /* ──── 参数页 ──── */
    case MENU_PARAM:
        if (key == 1) {
            const ParamDef *p = &param_table[param_cursor];
            int32_t v = *p->value + p->step;
            if (v > p->max) v = p->max;
            *p->value = v;
            draw_param();
        } else if (key == 2) {
            const ParamDef *p = &param_table[param_cursor];
            int32_t v = *p->value - p->step;
            if (v < p->min) v = p->min;
            *p->value = v;
            draw_param();
        } else if (key == 3) {
            param_cursor = (param_cursor + 1) % PARAM_COUNT;
            draw_param();
        } else if (key == 4) {
            page = MENU_SELECT;
            LCD_Fill(0, 0, LCD_W, LCD_H, BG_COLOR);
            draw_select();
        }
        break;

    /* ──── 运行中 ──── */
    case MENU_RUNNING:
        if (key == 3) {
            /* 暂停: 停电机, 冻结计时 */
            g_task_run = 0;
            g_paused = 1;
            g_pause_start = sys_tick;
            Motor_Control(1, 0);
            Motor_Control(2, 0);
            lcd_printf(0, TITLE_Y, RED, BG_COLOR, "  PAUSED  ");
        } else if (key == 4) {
            /* 停止: 回到菜单 */
            g_paused = 0;
            g_pause_accum = 0;
            TASK_Stop();
            page = MENU_SELECT;
            LCD_Fill(0, 0, LCD_W, LCD_H, BG_COLOR);
            draw_select();
        }
        break;

    /* ──── 暂停 ──── */
    case MENU_PAUSED:
        if (key == 3) {
            /* 恢复: 累计暂停时间 */
            g_pause_accum += sys_tick - g_pause_start;
            g_pause_start = 0;
            g_paused = 0;
            g_task_run = 1;
        } else if (key == 4) {
            /* 停止: 结算暂停时间, 回到菜单 */
            g_pause_accum += sys_tick - g_pause_start;
            g_paused = 0;
            TASK_Stop();
            page = MENU_SELECT;
            LCD_Fill(0, 0, LCD_W, LCD_H, BG_COLOR);
            draw_select();
        }
        break;
    }
}

static void draw_running(void)
{
    uint8_t idx = TASK_GetCurrentIdx();

    /* 如果任务有自定义显示，用它代替默认 */
    if (tasks[idx].draw) {
        tasks[idx].draw();
        return;
    }

    /* 默认显示：任务名 + 速度 + 传感器条 */
    lcd_printf(0, TITLE_Y, TITLE_COLOR, BG_COLOR, "%s RUN",
               task_names[idx]);
    lcd_printf(0, ITEM_Y0, TXT_COLOR, BG_COLOR,
               "Spd:%d L:%d R:%d", param_base_speed, speed_l, speed_r);
		
    lcd_printf(0, ITEM_Y0 + ITEM_H, TXT_COLOR, BG_COLOR, "Sns:");
    lcd_printf(0, HINT_Y, GRAY, BG_COLOR, "K3 Pause K4 Stop");
}

void MENU_Refresh(void)
{
    switch (page) {
    case MENU_PAUSED:
    case MENU_RUNNING:
        draw_running();
        break;
    default:
        break;
    }
    // 已由 draw_running() 统一刷新
		//lcd_printf(65, 20, GRAY, BG_COLOR,
      //         "yaw:%.1f", yaw);
		
}

static void draw_select(void)
{
    LCD_Fill(0, 0, LCD_W, 28, BG_COLOR);
    LCD_Fill(0, HINT_Y, LCD_W, LCD_H, BG_COLOR);
    lcd_printf(0, TITLE_Y, TITLE_COLOR, BG_COLOR, "Task Select");

    for (uint8_t i = 0; i < TASK_COUNT; i++) {
        uint16_t fg = (i == cursor) ? SEL_FG : TXT_COLOR;
        uint16_t bg = (i == cursor) ? SEL_BG : BG_COLOR;
        lcd_printf(15, ITEM_Y0 + i * ITEM_H, fg, bg, "%s", task_names[i]);
    }

    {
        uint8_t i = TASK_COUNT;
        uint16_t fg = (cursor == i) ? SEL_FG : TXT_COLOR;
        uint16_t bg = (cursor == i) ? SEL_BG : BG_COLOR;
        lcd_printf(15, ITEM_Y0 + i * ITEM_H, fg, bg, "Settings");
    }

    lcd_printf(0, HINT_Y, GRAY, BG_COLOR,
               "K1 Up K2 Dn K3 Go");
}

static void draw_param(void)
{
    LCD_Fill(0, 0, LCD_W, 28, BG_COLOR);
    LCD_Fill(0, HINT_Y, LCD_W, LCD_H, BG_COLOR);
    lcd_printf(0, TITLE_Y, TITLE_COLOR, BG_COLOR, "Settings");

    char buf[16];
    for (uint8_t i = 0; i < PARAM_COUNT; i++) {
        const ParamDef *p = &param_table[i];
        int32_t v = *p->value;

        if (p->fmt == FMT_INT)
            snprintf(buf, sizeof(buf), "%d", v);
        else
            snprintf(buf, sizeof(buf), "%d.%02d", v / 100, v % 100);

        uint16_t fg = (i == param_cursor) ? SEL_FG : TXT_COLOR;
        uint16_t bg = (i == param_cursor) ? SEL_BG : BG_COLOR;
        lcd_printf(10, ITEM_Y0 + i * ITEM_H, fg, bg, "%s: %s", p->name, buf);
    }

    lcd_printf(0, HINT_Y, GRAY, BG_COLOR,
               "K1+ K2- K3 Next K4 Back");
}
