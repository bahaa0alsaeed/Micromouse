#pragma once

struct PIDConfig
{
    float kp;
    float ki;
    float kd;
    float outputMin;
    float outputMax;
    float integralMin;
    float integralMax;
};

class PID
{
    PIDConfig config = {};
    float previousError = 0;
    float integral = 0;
    float output = 0;

public:
    explicit PID(const PIDConfig& config);
    float compute(float setpoint, float measurement, float deltaTime);
    void reset();
};

