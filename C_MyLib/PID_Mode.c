#include "PID_Mode.h"

// PID 初始化函数
void pid_init(PIDController* pid, double Kp, double Ki, double Kd, double setpoint) {
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->setpoint = setpoint;
    pid->integral = 0.0;
    pid->previous_error = 0.0;
}

// PID 计算函数
double pid_compute(PIDController* pid, double current_value, double dt) {
    // 计算误差
    double error = pid->setpoint - current_value;

    // 计算积分项
    pid->integral += error * dt;

    // 计算微分项
    double derivative = (error - pid->previous_error) / dt;

    // 计算 PID 输出
    double output = pid->Kp * error + pid->Ki * pid->integral + pid->Kd * derivative;

    // 更新上一次的误差
    pid->previous_error = error;

    return output;
}
