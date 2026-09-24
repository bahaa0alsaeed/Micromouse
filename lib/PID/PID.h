#pragma once
#include "Logger.h"

struct PIDConfig
{
    // PID controller gains.
    float kp = 0.0;
    float ki = 0.0;
    float kd = 0.0;

    // Smoothing factor used to filter the derivative term.
    float alpha = 0.7;

    // Output limits.
    float outputMin = 0.0;
    float outputMax = 0.0;

    // Integral limits used to prevent integral windup.
    float integralMin = 0.0;
    float integralMax = 0.0;
};

class PID
{
    PIDConfig config = {};

    // Internal state preserved between compute() calls.
    float previousError = 0.0;
    float integral = 0.0;
    float previousDerivative = 0.0;
    float output = 0.0;

public:
    explicit PID(const PIDConfig& config);

    // Calculate the PID output for the current measurement.
    float compute(float setpoint, float measurement, float dt);

    // Replace the current controller parameters.
    void changeParameters(const PIDConfig& newConfig);

    // Clear the controller's internal state.
    void reset();
};
