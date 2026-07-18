#ifndef __TRACKING_H
#define __TRACKING_H

#include <stdint.h>
#include <stdbool.h>

/**********************************************************
***  二维云台目标跟踪模块
***  视觉模块: MaixCAM2 (K230)
***  电机驱动: Emm_V5.0 闭环步进伺服 (Pan=地址1, Tilt=地址2)
***  主控芯片: MSPM0G3507 @ 32MHz (Cortex-M0+, 无FPU)
**********************************************************/

/* ================================================================
 * 相机参数 (MaixCAM2, 可按需修改)
 * ================================================================ */
#define CAMERA_WIDTH            320     /* 相机水平分辨率 */
#define CAMERA_HEIGHT           240     /* 相机垂直分辨率 */
#define CAMERA_CENTER_X         (CAMERA_WIDTH / 2)     /* 画面中心X = 160 */
#define CAMERA_CENTER_Y         (CAMERA_HEIGHT / 2)    /* 画面中心Y = 120 */

/* ================================================================
 * 电机参数 (可按需修改)
 * ================================================================ */
#define MOTOR_PAN_ADDR          1       /* 平移(水平)电机RS485地址 */
#define MOTOR_TILT_ADDR         2       /* 俯仰(垂直)电机RS485地址 */

#define MOTOR_VEL_RPM           800     /* 跟踪速度(RPM) */
#define MOTOR_ACC               5       /* 加速度(0-255, 0=瞬间到达) */
#define MOTOR_MIN_PULSES        5       /* 死区: 脉冲数小于此值忽略 */
#define MOTOR_MAX_PULSES        500     /* 安全限制: 单次最大脉冲数 */

/* 共享UART3总线的命令间隔(ms), 防止总线冲突 */
#define MOTOR_CMD_DELAY_MS      1

/* ================================================================
 * PID参数 (Q16.16 定点数格式: 实际值 * 65536)
 *
 * 像素→脉冲的坐标映射已合并到 Kp 中:
 *   脉冲/像素 = (脉冲/度) * (度/像素)
 *   = (3200/360) * (视场角 / 分辨率)
 *   = 8.889 * (60/320) = 1.667 脉冲/像素
 *
 * Kp = 1.667 * 65536 = 109227 (Q16格式)
 *
 * 以下为初始值, 需在硬件上调优.
 * ================================================================ */

/* 平移轴(Pan) PID增益 (Q16格式) */
#define PID_PAN_KP_Q16          109227  /* Kp = 1.667  (脉冲/像素) */
#define PID_PAN_KI_Q16          3277    /* Ki = 0.05   */
#define PID_PAN_KD_Q16          65536   /* Kd = 1.0    */

/* 俯仰轴(Tilt) PID增益 (Q16格式) — 通常比平移轴略低(受重力影响) */
#define PID_TILT_KP_Q16         109227  /* Kp = 1.667  */
#define PID_TILT_KI_Q16         6554    /* Ki = 0.10   (加大以对抗重力) */
#define PID_TILT_KD_Q16         65536   /* Kd = 1.0    */

/* 积分抗饱和限幅 (Q16格式) */
#define PID_INTEGRAL_MAX_Q16    (200 * 65536)

/* ================================================================
 * 跟踪参数
 * ================================================================ */
#define TRACKING_DEADZONE_PX    5       /* 像素死区: 小误差忽略 */
#define OBJ_LOST_TIMEOUT_MS     500     /* 超过500ms无有效帧则判定目标丢失 */

/* ================================================================
 * 视觉协议 (MaixCAM2 ASCII文本格式)
 * ================================================================ */
#define VISION_MAX_COORD_STR    64      /* 坐标字符串最大长度 */

/* ================================================================
 * 数据类型定义
 * ================================================================ */

/** PID控制器状态 (每轴一个) */
typedef struct {
    int32_t kp;             /* 比例增益 (Q16格式) */
    int32_t ki;             /* 积分增益 (Q16格式) */
    int32_t kd;             /* 微分增益 (Q16格式) */
    int32_t setpoint;       /* 目标值 (像素, 通常为画面中心) */
    int32_t integral;       /* 积分累积 (Q16格式) */
    int32_t prev_error;     /* 上一次误差 (原始像素) */
    int32_t integral_max;   /* 积分抗饱和限幅 (Q16格式) */
    int32_t output_max;     /* 输出限幅 (脉冲数) */
    uint8_t motor_addr;     /* 对应电机RS485地址 */
    uint8_t invert_dir;     /* 方向取反: 1=反转电机方向 */
} pid_controller_t;

/** 跟踪状态机状态 */
typedef enum {
    TRACK_STATE_IDLE = 0,       /* 空闲: 等待第一帧 */
    TRACK_STATE_SEARCHING,      /* 搜索: 已收到首帧, 正在建立锁定 */
    TRACK_STATE_LOCKED,         /* 锁定: 稳定跟踪中 */
    TRACK_STATE_LOST            /* 丢失: 目标丢失, 等待重新出现 */
} track_state_t;

/** 跟踪系统主状态结构体 */
typedef struct {
    track_state_t state;            /* 当前跟踪状态 */
    uint32_t last_frame_tick;       /* 最近一帧有效数据的时间戳(ms) */
    uint32_t frame_count;           /* 已处理的总帧数 */

    /* 解析得到的目标坐标 */
    int16_t target_x;
    int16_t target_y;
    bool    target_valid;

    /* 两轴PID控制器 */
    pid_controller_t pid_pan;
    pid_controller_t pid_tilt;

    /* 电机输出 (PID计算 → 转换为方向+绝对值) */
    int32_t motor_pan_pulses;       /* 平移电机脉冲数(绝对值) */
    int32_t motor_tilt_pulses;      /* 俯仰电机脉冲数(绝对值) */
    uint8_t motor_pan_dir;          /* 平移方向: 0=CW, 1=CCW */
    uint8_t motor_tilt_dir;         /* 俯仰方向: 0=CW, 1=CCW */
} tracking_system_t;

/* ================================================================
 * 公开API函数声明
 * ================================================================ */

/**
 * @brief  初始化跟踪系统
 *         设置PID控制器默认参数, 状态重置为IDLE
 * @param  sys  指向 tracking_system_t 的指针
 */
void tracking_init(tracking_system_t *sys);

/**
 * @brief  解析 MaixCAM2 发来的坐标字符串
 *         格式无关解析器: 提取字符串中前两个整数作为 X, Y 坐标
 *         支持格式: "X:123,Y:456", "123,456", "(123, 456)", "123 456"
 *         结果存入 sys->target_x, sys->target_y, sys->target_valid
 * @param  sys   指向跟踪系统的指针
 * @param  data  来自UART中断的原始字节缓冲区 (rxCmd[])
 * @param  len   缓冲区中有效字节数
 * @return true  成功提取并验证了两个坐标值
 */
bool tracking_parse_coords(tracking_system_t *sys, const uint8_t *data, uint8_t len);

/**
 * @brief  执行平移轴(Pan/X)的一次PID迭代
 *         误差 = CAMERA_CENTER_X - target_x (像素)
 *         输出存入 sys->motor_pan_pulses (带符号的脉冲数)
 * @param  sys  指向跟踪系统的指针
 */
void tracking_pid_pan(tracking_system_t *sys);

/**
 * @brief  执行俯仰轴(Tilt/Y)的一次PID迭代
 *         误差 = CAMERA_CENTER_Y - target_y (像素)
 *         输出存入 sys->motor_tilt_pulses (带符号的脉冲数)
 * @param  sys  指向跟踪系统的指针
 */
void tracking_pid_tilt(tracking_system_t *sys);

/**
 * @brief  将带符号的PID输出转换为方向 + 绝对值脉冲
 *         设置 sys->motor_pan_dir, motor_tilt_dir
 *         将 motor_pan_pulses, motor_tilt_pulses 转为绝对值
 * @param  sys  指向跟踪系统的指针
 */
void tracking_prepare_motor_commands(tracking_system_t *sys);

/**
 * @brief  通过UART3向两个电机发送位置控制命令
 *         仅当脉冲数超过 MOTOR_MIN_PULSES 时才发送
 *         使用 Emm_V5_Pos_Control() 的增量定位模式
 *         命令之间加入延时防止共享总线冲突
 * @param  sys  指向跟踪系统的指针
 */
void tracking_send_motor_commands(tracking_system_t *sys);

/**
 * @brief  更新跟踪状态机
 *         状态转移:
 *           IDLE      -> SEARCHING  (收到首个有效帧)
 *           SEARCHING -> LOCKED     (连续有效帧, 建立锁定)
 *           LOCKED    -> LOST       (超时无有效帧)
 *           LOST      -> SEARCHING  (有效帧重新出现)
 *         进入LOST状态时: 自动清零PID积分项, 防止积分饱和
 * @param  sys         指向跟踪系统的指针
 * @param  frame_valid 当前帧是否包含有效坐标
 * @param  tick_ms     当前系统tick (毫秒)
 */
void tracking_update_state(tracking_system_t *sys, bool frame_valid, uint32_t tick_ms);

/**
 * @brief  获取跟踪状态的字符串名称
 * @param  state  状态枚举值
 * @return 字符串字面量 ("IDLE", "SEARCH", "LOCKED", "LOST")
 */
const char* tracking_state_name(track_state_t state);

/**
 * @brief  刷新LCD显示跟踪状态信息
 *         显示内容: 状态, 目标坐标, 误差, 电机脉冲, 帧计数
 *         使用 lcd_printf() 函数 (lcd.h)
 * @param  sys  指向跟踪系统的指针
 */
void tracking_lcd_update(tracking_system_t *sys);

/**
 * @brief  获取系统毫秒 tick (自由运行计数器)
 *         由主应用程序实现 (在 empty.c 中定义)
 * @return 当前 tick 值 (ms)
 */
uint32_t tracking_get_tick_ms(void);

#endif /* __TRACKING_H */
