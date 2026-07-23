/**
 * ============================================================
 * IMU660RB (LSM6DSR) 移植模板
 * ============================================================
 *
 * 移植时需要修改的地方：
 *   1. 实现 SPI 读写单字节
 *   2. 实现 CS 拉低/拉高
 *   3. 实现毫秒延时
 *   4. 提供一个周期调用的接口（中断或定时器）
 *
 * 需要保留的文件（纯算法，无需修改）：
 *   - lsm6dsr_reg.c / lsm6dsr_reg.h     ← ST 官方寄存器驱动
 *   - Fusion                         ← Madgwick AHRS 算法
 *
 * ============================================================
 */

#include "imu660rb.h"
#include "lsm6dsr_reg.h"
#include "board.h"
#include <stdint.h>

#define BOOT_TIME         (10)
#define OFFSET_CAL_TIME   (50)

#define ODR_COEFF_12Hz5   (512)
#define ODR_COEFF_26Hz    (256)
#define ODR_COEFF_52Hz    (128)
#define ODR_COEFF_104Hz   (64)
#define ODR_COEFF_208Hz   (32)
#define ODR_COEFF_416Hz   (16)
#define ODR_COEFF_833Hz   (8)
#define ODR_COEFF_1667Hz  (4)
#define ODR_COEFF_3333Hz  (2)
#define ODR_COEFF_6667Hz  (1)

/* ========== 配置宏（根据你的平台修改）========== */

/* SPI 时钟频率建议 1~10 MHz，3-Wire Mode 0 (CPOL=0, CPHA=0) */
/* 数据格式：MSB first, 8-bit */

/* ========== 平台相关函数声明（需要你实现）========== */

/**
 * @brief SPI 收发一个字节（3-Wire 半双工）
 *        发送 data，同时接收一个字节返回
 */
static uint8_t spi_transfer_byte(uint8_t data);

/**
 * @brief 拉低 CS 片选（开始通信）
 */
static void cs_low(void);

/**
 * @brief 拉高 CS 片选（结束通信）
 */
static void cs_high(void);

/**
 * @brief 毫秒延时
 */
static void delay_MS(uint32_t ms);

/* ========== 全局变量 ========== */

static stmdev_ctx_t dev_ctx;

float acceleration_mg[3];      // 加速度，单位 mg
float angular_rate_mdps[3];    // 角速度，单位 mdps (毫度/秒)

static int16_t data_raw_acceleration[3];
static int16_t data_raw_angular_rate[3];
static uint8_t whoamI, rst;

/* Fusion AHRS */
FusionAhrs ahrs;
FusionEuler euler;
static FusionOffset offset;
static float sample_period;

/* 陀螺仪校准参数（初始化时自动计算）*/
static FusionMatrix gyro_misalignment = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
static FusionVector gyro_sensitivity = {1.0f, 1.0f, 1.0f};
static FusionVector gyro_offset = {0.0f, 0.0f, 0.0f};

/* ========== 平台 SPI 读写层（接入 lsm6dsr_reg 回调）========== */

static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len)
{
    cs_low();
    spi_transfer_byte(reg);          // 发送寄存器地址（写，bit7=0）
    while (len--) {
        spi_transfer_byte(*bufp++);  // 发送数据
    }
    cs_high();
    return 0;
}

static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len)
{
    cs_low();
    reg |= 0x80;                     // 读操作：bit7 置 1
    spi_transfer_byte(reg);          // 发送寄存器地址
    while (len--) {
        *bufp++ = spi_transfer_byte(0);  // 发送 dummy 同时接收
    }
    cs_high();
    return 0;
}

static void platform_delay(uint32_t ms)
{
    delay_MS(ms);
}

/* ========== 初始化 ========== */

int IMU660RB_Init(void)
{
    lsm6dsr_pin_int1_route_t int1_route;
    uint8_t offset_cnt;
    int8_t freq_fine;
    float sample_rate;

    /* 1. 注册 SPI 接口 */
    dev_ctx.write_reg = platform_write;
    dev_ctx.read_reg  = platform_read;
    dev_ctx.mdelay    = platform_delay;

    /* 2. 等待传感器启动 */
    delay_ms(10);

    /* 3. 检查器件 ID */
    lsm6dsr_device_id_get(&dev_ctx, &whoamI);
    if (whoamI != LSM6DSR_ID) {
        return -1;  // 未检测到 IMU660RB
    }

    /* 4. 恢复默认配置 */
    lsm6dsr_reset_set(&dev_ctx, PROPERTY_ENABLE);
    do {
        lsm6dsr_reset_get(&dev_ctx, &rst);
    } while (rst);

    /* 5. 关闭 I3C */
    lsm6dsr_i3c_disable_set(&dev_ctx, LSM6DSR_I3C_DISABLE);

    /* 6. 使能 Block Data Update（防止读写撕裂）*/
    lsm6dsr_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);

    /* 7. 设置输出数据率（建议 52Hz ~ 208Hz）*/
    lsm6dsr_xl_data_rate_set(&dev_ctx, LSM6DSR_XL_ODR_52Hz);
    lsm6dsr_gy_data_rate_set(&dev_ctx, LSM6DSR_GY_ODR_52Hz);

    /* 8. 设置量程 */
    lsm6dsr_xl_full_scale_set(&dev_ctx, LSM6DSR_2g);       // ±2g
    lsm6dsr_gy_full_scale_set(&dev_ctx, LSM6DSR_2000dps);  // ±2000dps

    /* 9. 开启陀螺仪 LPF */
    lsm6dsr_gy_filter_lp1_set(&dev_ctx, 1);

    /* 10. 配置 INT1 引脚：数据就绪时触发中断 */
    lsm6dsr_pin_int1_route_get(&dev_ctx, &int1_route);
    int1_route.int1_ctrl.int1_drdy_xl = PROPERTY_ENABLE;
    lsm6dsr_pin_int1_route_set(&dev_ctx, &int1_route);
    lsm6dsr_data_ready_mode_set(&dev_ctx, LSM6DSR_DRDY_PULSED);

    /* 11. 计算采样周期 */
    lsm6dsr_odr_cal_reg_get(&dev_ctx, &freq_fine);
    sample_rate = (6667.0f + (0.0015f * freq_fine) * 6667.0f) / 128.0f;  // ÷128 for 52Hz
    sample_period = 1.0f / sample_rate;

    /* 12. 初始化 AHRS 和 Offset 滤波器 */
    FusionAhrsInitialise(&ahrs);
    FusionOffsetInitialise(&offset, sample_rate);

    delay_ms(200);

    /* 13. 静止校准陀螺零偏（采集 50 次取平均）*/
    offset_cnt = 50;
    gyro_offset.array[0] = 0;
    gyro_offset.array[1] = 0;
    gyro_offset.array[2] = 0;

    while (offset_cnt) {
        uint8_t reg;
        lsm6dsr_gy_flag_data_ready_get(&dev_ctx, &reg);
        if (reg) {
            offset_cnt--;
            lsm6dsr_angular_rate_raw_get(&dev_ctx, data_raw_angular_rate);
            angular_rate_mdps[0] = lsm6dsr_from_fs2000dps_to_mdps(data_raw_angular_rate[0]);
            angular_rate_mdps[1] = lsm6dsr_from_fs2000dps_to_mdps(data_raw_angular_rate[1]);
            angular_rate_mdps[2] = lsm6dsr_from_fs2000dps_to_mdps(data_raw_angular_rate[2]);
            gyro_offset.array[0] += angular_rate_mdps[0] / 1000.0f;
            gyro_offset.array[1] += angular_rate_mdps[1] / 1000.0f;
            gyro_offset.array[2] += angular_rate_mdps[2] / 1000.0f;
        }
    }
    gyro_offset.array[0] /= 50.0f;
    gyro_offset.array[1] /= 50.0f;
    gyro_offset.array[2] /= 50.0f;

    return 0;  // 成功
}

/* ========== 数据读取（每次数据就绪时调用）========== */

void Read_IMU660RB(void)
{
    /* 读加速度计原始值 */
    lsm6dsr_acceleration_raw_get(&dev_ctx, data_raw_acceleration);
    acceleration_mg[0] = lsm6dsr_from_fs2g_to_mg(data_raw_acceleration[0]);
    acceleration_mg[1] = lsm6dsr_from_fs2g_to_mg(data_raw_acceleration[1]);
    acceleration_mg[2] = lsm6dsr_from_fs2g_to_mg(data_raw_acceleration[2]);

    /* 读陀螺仪原始值 */
    lsm6dsr_angular_rate_raw_get(&dev_ctx, data_raw_angular_rate);
    angular_rate_mdps[0] = lsm6dsr_from_fs2000dps_to_mdps(data_raw_angular_rate[0]);
    angular_rate_mdps[1] = lsm6dsr_from_fs2000dps_to_mdps(data_raw_angular_rate[1]);
    angular_rate_mdps[2] = lsm6dsr_from_fs2000dps_to_mdps(data_raw_angular_rate[2]);

    /* 构建 Fusion 向量（转单位为 g 和 dps）*/
    FusionVector accel = {
        acceleration_mg[0] / 1000.0f,
        acceleration_mg[1] / 1000.0f,
        acceleration_mg[2] / 1000.0f
    };
    FusionVector gyro = {
        angular_rate_mdps[0] / 1000.0f,
        angular_rate_mdps[1] / 1000.0f,
        angular_rate_mdps[2] / 1000.0f
    };

    /* 校准陀螺仪 */
    gyro = FusionCalibrationInertial(gyro, gyro_misalignment, gyro_sensitivity, gyro_offset);
    gyro = FusionOffsetUpdate(&offset, gyro);

    /* AHRS 融合（无磁力计版本）*/
    FusionAhrsUpdateNoMagnetometer(&ahrs, gyro, accel, sample_period);

    /* 四元数 → 欧拉角（pitch, roll, yaw）*/
    euler = FusionQuaternionToEuler(FusionAhrsGetQuaternion(&ahrs));
}

/* ============================================================
 * 以下是你需要在平台上实现的函数：
 *
 *   1. spi_transfer_byte(uint8_t data)  —— SPI 收发一个字节
 *   2. cs_low() / cs_high()             —— 控制 CS 片选引脚
 *   3. delay_ms(uint32_t ms)            —— 毫秒延时
 *
 * 调用方式：
 *   - 初始化：IMU660RB_Init() 调用一次
 *   - 读取：每次数据就绪时调用 Read_IMU660RB()
 *           （可用 INT1 中断触发，或用定时器按 ODR 频率轮询）
 *   - 获取航向：euler.angle.yaw / pitch / roll
 *
 * SPI 时序要求：
 *   - 3-Wire Motorola 模式（半双工，MISO/MOSI 共用一根线）
 *   - CPOL=0, CPHA=0 (Mode 0)
 *   - MSB first, 8-bit
 *   - 时钟频率建议 ≤ 10 MHz
 *   - CS 低电平时开始传输，高电平时结束
 * ============================================================ */


/**
 * @brief SPI 收发一个字节（3-Wire 半双工）
 *        发送 data，同时接收一个字节返回
 */
static uint8_t spi_transfer_byte(uint8_t data)
{
	uint8_t read_data = 0;

    DL_SPI_transmitData8(SPI_IMU660RB_INST, data);
    while(DL_SPI_isRXFIFOEmpty(SPI_IMU660RB_INST));
    read_data = DL_SPI_receiveData8(SPI_IMU660RB_INST);
    while(DL_SPI_isBusy(SPI_IMU660RB_INST));

    return read_data;
}

/**
 * @brief 拉低 CS 片选（开始通信）
 */
static void cs_low(void)
{
	DL_GPIO_clearPins(GPIO_IMU660RB_PIN_IMU660RB_INT1_PORT, GPIO_IMU660RB_PIN_IMU660RB_CS_PIN);
}

/**
 * @brief 拉高 CS 片选（结束通信）
 */
static void cs_high(void)
{
	DL_GPIO_setPins(GPIO_IMU660RB_PIN_IMU660RB_INT1_PORT, GPIO_IMU660RB_PIN_IMU660RB_CS_PIN);
}

/**
 * @brief 毫秒延时
 */
static void delay_MS(uint32_t ms)
{
	mspm0_delay_ms(ms);

}