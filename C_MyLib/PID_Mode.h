#ifndef __PID_MODE_H__
#define __PID_MODE_H__

// PID 控制器结构体
typedef struct {
    double Kp;             // 比例系数
    double Ki;             // 积分系数
    double Kd;             // 微分系数
    double setpoint;       // 目标值
    double integral;       // 积分项
    double previous_error; // 上一次的误差
} PIDController;

extern void pid_init(PIDController* pid, double Kp, double Ki, double Kd, double setpoint);
extern double pid_compute(PIDController* pid, double current_value, double dt);

#endif
