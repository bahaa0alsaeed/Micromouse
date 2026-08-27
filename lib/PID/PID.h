#pragma once

struct PIDConfig
{
    float kp = 0.0;
    float ki = 0.0;
    float kd = 0.0;
    float alpha = 0.7;
    float outputMin = 0.0;
    float outputMax = 0.0;
    float integralMin = 0.0;
    float integralMax = 0.0;
};

class PID
{
    PIDConfig config = {};

    float previousError = 0.0;
    float integral = 0.0;
    float previousDerivative = 0.0;
    float output = 0.0;

public:
    explicit PID(const PIDConfig& config);
    float compute(float setpoint, float measurement, float dt);
    void changeParameters(const PIDConfig& newConfig);
    void reset();
};

