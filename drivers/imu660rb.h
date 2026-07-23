#ifndef _IMU660RB_PORT_H_
#define _IMU660RB_PORT_H_

#include "Fusion.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 加速度（mg）和角速度（mdps）*/
extern float acceleration_mg[3];
extern float angular_rate_mdps[3];

/* AHRS 实例和欧拉角输出 */
extern FusionAhrs ahrs;
extern FusionEuler euler;

/* 初始化，成功返回 0，失败返回 -1 */
int  IMU660RB_Init(void);

/* 读取传感器并更新姿态（每次数据就绪时调用一次）*/
void Read_IMU660RB(void);

#ifdef __cplusplus
}
#endif

#endif
