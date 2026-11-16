
#pragma once

#include <limits>

// Identifies configuration and runtime errors.
enum class PIDError
{
    None,
    NotInitialized,
    InvalidGains,
    InvalidFilterParameter,
    InvalidOutputLimits,
    InvalidIntegralLimits,
    InvalidSampleTime,
    InvalidSetpoint,
    InvalidMeasurement,
    InvalidFeedforward,
    NumericalError
};

struct PIDConfig
{
    // PID controller gains.
    float kp = 0.0f;
    float ki = 0.0f;
    float kd = 0.0f;

    // Derivative filter coefficient (1/seconds).
    float N = 0.0f;

    // Output limits must be configured before begin().
    float outputMin = std::numeric_limits<float>::quiet_NaN();
    float outputMax = std::numeric_limits<float>::quiet_NaN();

    // Integral limits used to prevent integral windup.
    float integralMin = 0.0f;
    float integralMax = 0.0f;
};

class PID
{
    PIDConfig config = {};

    // Internal state preserved between compute() calls.
    float previousMeasurement = 0.0f;
    float integral = 0.0f;
    float previousDerivative = 0.0f;
    float output = 0.0f;

    // Controller initialization and validation state.
    bool initialized = false;
    bool configured = false;

    // Stores the most recent controller error.
    PIDError lastError = PIDError::NotInitialized;

    // Handle runtime errors and reset control history.
    float fail(PIDError error);

public:
    explicit PID(const PIDConfig& config);

    // Validate the controller configuration.
    bool begin();

    // Calculate the PID output with optional external feedforward.
    float compute(float setpoint, float measurement,
                  float dt, float feedforward = 0.0f);

    // Replace the controller parameters.
    void changeParameters(const PIDConfig& newConfig);

    // Reset the controller's internal state.
    void reset();

    // Retrieve the latest error code.
    PIDError getLastError() const;

    // Retrieve a readable error message.
    const char* getErrorMessage() const;
};
