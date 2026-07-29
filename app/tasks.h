#ifndef _TASKS_H_
#define _TASKS_H_

#include <stdint.h>
#include "params.h"

/* ── 任务结构体 ──
 *  init()  : 选中任务时调用一次（重置变量用）
 *  run()   : 主循环每次 while 都调一次（控制逻辑写这里）
 *            sensor → bin_array[12]，tick_ms → sys_tick
 *  stop()  : 退出任务时调用一次（清变量、关输出）
 *  draw()  : 自定义 LCD 显示（NULL=用默认显示，包含速度和传感器条）
 *
 *  ★ run() 内部节奏控制方式（选一种）：
 *    方式A 等传感器：if (!bin_ready) return; bin_ready = 0;
 *    方式B 定时执行：if (tick_ms - last < 20) return; last = tick_ms;
 *    方式C 全力跑 ：不等待，每次 while 都执行
 */
typedef struct {
    void (*init)(void);
    void (*run)(uint8_t *sensor, uint32_t tick_ms);
    void (*stop)(void);
    void (*draw)(void);
} Task;

/* ── 全局变量（主循环用） ──
 *  tasks[TASK_COUNT]  任务注册表（在 tasks.c 底部）
 *  g_active_task      当前运行的任务指针，NULL=无任务
 *  g_task_run         1=运行中，0=暂停（K3切换）
 */
extern const Task tasks[TASK_COUNT];
extern const Task *g_active_task;
extern volatile uint8_t g_task_run;

/* ── 任务管理 API ──
 *  TASK_Select(idx) : 选 idx 号任务→调init→g_task_run=1→主循环开始跑run()
 *  TASK_Stop()      : 停当前任务→调stop→g_task_run=0→电机停
 *  TASK_GetCurrentIdx() : 返回当前任务索引（LCD显示用）
 */
void TASK_Select(uint8_t idx);
void TASK_Stop(void);
uint8_t TASK_GetCurrentIdx(void);

#endif
