# MSPM0G3507 循迹小车 — 双闭环PID + 多任务状态机

基于 **TI MSPM0G3507** (Cortex-M0+, 32MHz) 的智能循迹小车系统，实现 12 路灰度传感器采集 → PD 循迹转向 + 增量式 PID 速度闭环 → AT8236 双电机驱动。

## 功能特性

- **12 路灰度传感器**: UART0 帧协议 (`#` 帧头 + 12bit ASCII + `!` 帧尾)，实时黑线位置检测
- **双闭环 PID 控制**:
  - **速度环**: 编码器反馈增量式 PID (KP/KD)，2ms 控制周期
  - **循迹环**: 加权重心 PD 转向 (KP/KD 可调)，10ms 控制周期，丢线自动恢复
- **3 任务系统**: 状态机架构，支持软启动/缓加速/降速停车
  - Task 1 `Trail`: 循迹 + 一圈计时 + 停止线检测停车
  - Task 2 `A→B`: 循迹 + 编码器 4000 脉冲停车
  - Task 3 `A→A`: 循迹 + 编码器 12000 脉冲停车 (倍率斜坡缓启动)
- **LCD 交互菜单**: ST7789 240×240 SPI LCD，4 页面状态机 (SELECT / RUNNING / PAUSED / PARAM)
- **实时参数调节**: 运行中可调速度、循迹 KP/KD，无需重新编译
- **暂停/恢复**: K3 暂停冻结计时 + 电机停止，K3 恢复累计计时，K4 停止回菜单
- **IMU 姿态**: IMU660RB / LSM6DSR 陀螺仪支持，Fusion AHRS 姿态解算 (yaw 航向角)
- **AT8236 电机驱动**: PWM 双通道 (左/右)，-999~999 速度范围

## 硬件清单

| 器件 | 型号/规格 | 说明 |
|------|-----------|------|
| 主控 | MSPM0G3507 | Cortex-M0+, 32MHz, 32KB SRAM |
| 电机驱动 | AT8236 | 双路 PWM 控制 |
| 电机 | 带编码器直流减速电机 ×2 | 左轮/右轮独立驱动 |
| 传感器 | 12 路灰度传感器阵列 | UART0 帧协议通信 |
| 显示 | ST7789 | 240×240 SPI LCD |
| IMU | IMU660RB / LSM6DSR | SPI 接口，6 轴姿态 |
| 按键 | 4 键 (K1~K4) | 上/下/确认/返回 |

## 目录结构

```
├── app/                    # 应用层
│   ├── main.c/h            # 主程序入口、系统心跳、中断处理
│   ├── tasks.c/h           # 3任务定义 + 任务管理API（Select/Stop）
│   ├── menu.c/h            # 4页面菜单状态机（SELECT/RUNNING/PAUSED/PARAM）
│   └── params.c/h          # 可调参数表（Speed, T3Spd, TrlKP, TrlKD）
├── bsp/                    # 板级支持包
│   ├── board.c/h           # 硬件初始化、延时、UART、按键扫描
│   ├── interrupt.c/h       # 中断向量配置 (GPIO编码器脉冲捕获)
│   └── uart_vision.c/h     # UART 视觉通信 (MaixCAM2 兼容, 保留)
├── config/                 # TI-DL 驱动库配置
│   └── ti_msp_dl_config.c/h  # SysConfig 外设初始化
├── drivers/                # 驱动层
│   ├── trailing.c/h        # 12路灰度解析 + PD循迹转向 + 停止线检测
│   ├── lcd.c/h             # ST7789 LCD 图形函数 + 字体
│   ├── lcd_port.c/h        # LCD SPI 端口抽象
│   ├── lcd_front.h         # 颜色/字体定义
│   ├── imu660rb.c/h        # IMU660RB 陀螺仪驱动 (SPI)
│   ├── lsm6dsr_reg.c/h     # LSM6DSR 陀螺仪寄存器驱动
│   └── Fusion/             # xioTechnologies Fusion AHRS 姿态解算库
├── motor/                  # 电机驱动
│   └── Motor.c/h           # AT8236 PWM控制 + 增量式PID速度闭环
├── keil/                   # Keil MDK 项目文件
├── startup/                # 启动代码
├── empty.syscfg            # TI SysConfig 配置文件
├── .gitignore
└── README.md
```

## 系统架构

### 主循环双轨设计

```
while(1) {
    高速控制轨: task->run() 每次循环都执行  (PD循迹 + 速度闭环)
    低速界面轨: 每10ms执行一次             (按键扫描 + LCD刷新)
}
```

### 控制链路

```
12路灰度 ─UART0→ 帧解析(#...!) → bin_array[12]
    ↓
Trail_Steering_Compute: 加权重心 → PD → 差速值
    ↓
Motor_SpeedLoop: 编码器反馈 → 增量式PID → PWM占空比
    ↓
AT8236 → 左右电机
```

### 菜单状态机

```
SELECT ─K3→ RUNNING ─K3→ PAUSED
  ↑           ↓ K4        ↓ K4
  └───────────┴───────────┘ → SELECT
  ↓ K4 (Settings项)
PARAM → K4 → SELECT
```

### 按键约定

| 按键 | 菜单页 | 参数页 | 运行中 |
|------|--------|--------|--------|
| K1 | 上移 | 参数+ | (无效) |
| K2 | 下移 | 参数- | (无效) |
| K3 | 确认/进入 | 下一项 | 暂停/恢复 |
| K4 | (无效)  | 返回菜单 | 停止 |

## 构建

### Keil MDK

1. 打开 `keil/empty_LP_MSPM0G3507_nortos_keil.uvprojx`
2. 编译 (Build → Build Project)
3. 烧录到 MSPM0G3507

### TI SysConfig

修改 `empty.syscfg` 后，在 Keil 中重新构建以生效。

## 使用

1. **上电**: LCD 显示 `Task Select` 菜单
2. **选任务**: K1/K2 上下选择，K3 确认启动
3. **调参数**: 选 `Settings` → K3 进入参数页，K1+/K2- 调节，K3 切换参数项
4. **运行中**: LCD 实时显示编码器进度、速度、计时
5. **暂停**: K3 暂停 (电机停止、计时冻结)
6. **停止**: K4 停止回菜单

## 可调参数

| 参数 | 默认值 | 范围 | 说明 |
|------|--------|------|------|
| Speed | 600 | 0~3000 | 任务1 基础速度 (编码器单位) |
| T3 Spd | 470 | 0~3000 | 任务3 基础速度 |
| Trl KP | 18.50 (×100) | 0~100.00 | 循迹比例增益 |
| Trl KD | 2.00 (×100) | 0~50.00 | 循迹微分增益 |

## PID 参数说明

### 速度闭环 (Motor.c)
- `KP_SPEED = 0.8`: 比例增益，调响应速度
- `KI_SPEED = 0.15`: 积分增益，消除稳态误差
- `KD_SPEED = 0.1`: 微分增益，抑制超调
- 控制周期: 2ms，增量式 PID

### 循迹转向 (trailing.c)
- `param_trail_kp`: 比例增益 (可调)，控制转向力度
- `param_trail_kd`: 微分增益 (可调)，抑制蛇形震荡
- 控制周期: 10ms，12路加权重心 (权重: -24~+24)

## 依赖

- TI MSPM0 SDK v2.02.00.05
- Keil MDK 5 (ARMCC)
- TI SysConfig
- [Fusion AHRS](https://github.com/xioTechnologies/Fusion) (姿态解算)
