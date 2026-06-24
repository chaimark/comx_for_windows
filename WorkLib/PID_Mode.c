#include "PID_Mode.h"

// PID 计算函数
double _pid_compute(struct _PIDController This, double NowValue, double dt) {
    // 计算误差
    double error = This.setpoint - NowValue;

    // 计算积分项
    This.integral += error * dt;

    // 计算微分项
    double derivative = (error - This.previous_error) / dt;

    // 计算 PID 输出
    double output = This.Kp * error + This.Ki * This.integral + This.Kd * derivative;

    // 更新上一次的误差
    This.previous_error = error;

    return output;
}

// 初始化 PID 控制结构体
PIDController pid_init(double Kp, double Ki, double Kd, double setpoint) {
    PIDController pid = {0};
    pid.Kp = Kp;
    pid.Ki = Ki;
    pid.Kd = Kd;
    pid.integral = 0.0;
    pid.setpoint = setpoint;
    pid.previous_error = 0.0;
    pid.pid_compute = _pid_compute;
    return pid;
}
