#ifndef _MENU_H_
#define _MENU_H_

#include <stdint.h>

/* ── 状态机：4个页面 ──
 *  MENU_SELECT  主菜单         K1/K2选任务  K3进入
 *  MENU_PARAM   参数设置        K1+  K2-  K3下一项  K4返回
 *  MENU_RUNNING 任务运行中      K3暂停  K4停止退出
 *  MENU_PAUSED  暂停            K3继续  K4停止退出
 */
typedef enum {
    MENU_SELECT,
    MENU_PARAM,
    MENU_RUNNING,
    MENU_PAUSED,
} MenuPage;

/* ── 初始化 ──
 *  上电调一次，进 SELECT 页面
 */
void MENU_Init(void);

/* ── 按键处理 ──
 *  每10ms调一次（main循环里已安排）
 *  key = 1~4 对应 K1~K4
 */
void MENU_KeyHandler(uint8_t key);

/* ── 画面刷新 ──
 *  每10ms调一次，负责更新传感器条
 */
void MENU_Refresh(void);


extern float yaw;
#endif
