#ifndef _PARAMS_H_
#define _PARAMS_H_

#include <stdint.h>

/* ── 任务数量 & 名字长度 ── */
#define TASK_COUNT  3
#define NAME_LEN    8

/* ── 任务名称（K4可改，重启恢复默认） ── */
extern char task_names[TASK_COUNT][NAME_LEN + 1];

/* ── ★ 可调参数（菜单 Settings 页修改） ──
 *
 *  在你的 task_run() 里直接读这些变量，不用传参
 *  Settings → K1+  K2-  K3下一项  K4返回
 *
 *  常用场景：
 *    param_base_speed  基础速度
 *    param_turn_speed  转弯速度
 *    param_pid_p       P 系数（×100，150→1.50）
 *    param_pid_i       I 系数
 *    param_pid_d       D 系数
 */
extern int32_t param_base_speed;
extern int32_t param_turn_speed;
extern int32_t param_pid_p;
extern int32_t param_pid_i;
extern int32_t param_pid_d;
extern int32_t param_lap_enc;
#define PARAM_COUNT 6

/* ── 参数显示格式 ──
 *  FMT_INT  : 直接显示整数，如 500
 *  FMT_DEC2 : 显示 ÷100，如 150→"1.50"
 */
typedef enum {
    FMT_INT,
    FMT_DEC2,
} ParamFmt;

/* ── 参数定义表结构 ──
 *  参数入口在 params.c 底部
 *  新增参数：
 *    1. 在 params.h 加 extern int32_t param_xxx;
 *    2. 在 params.c 加 int32_t param_xxx = 初值;
 *    3. 在 param_table 加一行
 *    4. 更新 PARAM_COUNT
 */
typedef struct {
    const char *name;
    int32_t *value;
    int32_t min;
    int32_t max;
    int32_t step;
    ParamFmt fmt;
} ParamDef;

extern const ParamDef param_table[PARAM_COUNT];

void PARAM_Init(void);

#endif
