# 二维云台目标跟踪系统

基于 **TI MSPM0G3507** 的智能云台，实现视觉目标检测 → PID控制 → 电机驱动闭环跟踪。

## 功能特性

- **双源视觉输入** (自动优先切换):
  - UART3 (115200bps): MaixCAM2 K230 真实视觉模块
  - UART0 (9600bps): PC模拟视觉 (USB转串口)
- **双轴闭环控制**: Emm_V5.0 步进伺服 × 2 (Pan=地址1, Tilt=地址2)
- **PID定点控制**: Q16.16 定点数格式, 支持方向反转与积分抗饱和
- **状态机管理**: IDLE → SEARCHING → LOCKED → LOST 四状态自动切换
- **LCD实时显示**: ST7735 128×160 SPI LCD 显示状态、坐标、误差、帧数

## 硬件清单

| 器件 | 型号/规格 | 说明 |
|------|-----------|------|
| 主控 | MSPM0G3507 | Cortex-M0+, 32MHz |
| 电机 | Emm_V5.0 闭环步进伺机 | 2个 (Pan/Tilt) |
| 视觉 | MaixCAM2 (K230) | 320×240, 或PC模拟 |
| 显示 | ST7735S | 128×160 SPI LCD |
| 电源 | 5V/12V | 视觉模块与电机分别供电 |

## 目录结构

```
├── app/              # 应用层
│   ├── main.c        # 主程序 (初始化、视觉解析、PID循环)
│   ├── tracking.c    # 跟踪算法实现 (PID、状态机、LCD显示)
│   └── tracking.h    # 跟踪模块头文件
├── bsp/              # 板级支持包
│   ├── board.c/h     # 硬件初始化、延时、UART收发
│   ├── uart_vision.c/h  # UART3视觉数据接收
├── config/           # TI-DL配置文件
│   ├── ti_msp_dl_config.c/h  # 外设初始化 (由SysConfig生成)
├── drivers/          # 驱动层
│   ├── lcd.c/h       # ST7735 LCD图形函数
│   ├── lcd_port.c/h  # LCD SPI端口抽象
│   └── lcd_front.h   # 颜色/字体定义
├── motor/            # 电机驱动
│   └── Emm_V5.c/h    # Emm_V5.0闭环步进伺机协议
├── startup/          # 启动代码
│   └── startup_mspm0g350x_uvision.s
├── keil/             # Keil MDK项目文件
├── empty.syscfg      # SysConfig配置文件
├── .gitignore
└── README.md
```

## 构建

### Keil MDK

1. 打开 `keil/empty_LP_MSPM0G3507_nortos_keil.uvprojx`
2. 编译 (Build → Build Project)
3. 烧录到 MSPM0G3507

### TI SysConfig

修改 `empty.syscfg` 后, 在Keil中重新构建项目以生效。

## 使用

1. **连接硬件**:
   - UART0 (PA10/PA11): PC模拟视觉 (可选)
   - UART3 (PB2/PB3): MaixCAM2视觉模块
   - LCD SPI: ST7735 128×160
   - 电机RS485: Pan(地址1), Tilt(地址2)

2. **视觉协议**:
   - MaixCAM2发送格式: `X:123,Y:456` (ASCII)
   - PC模拟格式: `123,456` (ASCII)
   - 坐标原点为左上角, 中心为(160, 120)

3. **烧录运行**:
   - LCD显示 `== Gimbal Tracker ==`
   - 电机使能后进入跟踪循环
   - 实时显示跟踪状态、目标坐标、电机脉冲数

## 配置参数

| 参数 | 默认值 | 说明 |
|------|--------|------|
| CAMERA_WIDTH/HEIGHT | 320/240 | 相机分辨率 |
| MOTOR_VEL_RPM | 800 | 电机转速 |
| MOTOR_ACC | 5 | 电机加速度 |
| PID_PAN/TILT_KP | 109227 (Q16) | 比例增益 |
| TRACKING_DEADZONE_PX | 5 | 像素死区 |
| OBJ_LOST_TIMEOUT_MS | 500 | 目标丢失超时 |

## 依赖

- TI MSPM0 SDK v2.02.00.05
- Keil MDK 5 (ARMCC)
- TI SysConfig
