#ifndef __PID_MODE_H__
#define __PID_MODE_H__

#include "StrLib.h"
// PID 控制器结构体
typedef struct _PIDController {
    double Kp;             // 比例系数
    double Ki;             // 积分系数
    double Kd;             // 微分系数
    double setpoint;       // 目标值
    double integral;       // 积分值
    double previous_error; // 上一次的误差
    double (*pid_compute)(struct _PIDController This, double NowValue, double dt);
} PIDController;

extern PIDController pid_init(double Kp, double Ki, double Kd, double setpoint);

#endif
