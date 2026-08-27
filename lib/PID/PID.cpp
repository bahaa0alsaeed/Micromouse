#include "PID.h"

PID::PID(const PIDConfig& config)
{
    PID::config = config;
}

float PID::compute(const float setpoint, const float measurement, float dt)
{
    const float error = setpoint - measurement;

    // Accumulate and clamp integral to prevent windup
    integral += error * dt;
    if (integral > config.integralMax) integral = config.integralMax;
    else if (integral < config.integralMin) integral = config.integralMin;

    // Apply low-pass filter to derivative to reduce noise
    if (dt == 0) dt = 0.001;
    const float raw_derivative = (error - previousError) / dt;
    const float derivative = config.alpha * previousDerivative +
        (1 - config.alpha) * raw_derivative;

    previousDerivative = derivative;
    previousError = error;

    // Combine P, I, and D components
    const float raw_output = config.kp * error + config.ki * integral + config.kd * derivative;

    if (raw_output > config.outputMax)
        output = config.outputMax;
    else if (raw_output < config.outputMin)
        output = config.outputMin;
    else
        output = raw_output;

    return output;
}

void PID::changeParameters(const PIDConfig& newConfig)
{
    config = newConfig;
}

void PID::reset()
{
    previousDerivative = 0.0;
    previousError = 0.0;
    output = 0.0;
    integral = 0.0;
}