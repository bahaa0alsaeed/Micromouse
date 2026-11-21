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

// Contains the output and status of a compute() call.
struct PIDResult
{
    float output;
    bool success;
    PIDError error;
};

struct PIDConfig
{
    // PID controller gains.
    float kp = 0.0f;
    float ki = 0.0f;
    float kd = 0.0f;

    // Derivative filter coefficient (1/seconds).
    float N = 0.0f;

    // Output limits must be explicitly configured.
    float outputMin = std::numeric_limits<float>::quiet_NaN();
    float outputMax = std::numeric_limits<float>::quiet_NaN();

    // Integral limits used to prevent integral windup.
    float integralMin = 0.0f;
    float integralMax = 0.0f;
};

class PID
{
    PIDConfig config = {};

    // State saved only after a successful compute() call.
    float previousMeasurement = 0.0f;
    float integral = 0.0f;
    float previousDerivative = 0.0f;
    float output = 0.0f;

    bool initialized = false;
    bool configured = false;

    PIDError lastError = PIDError::NotInitialized;

    // Configuration and input validation.
    static PIDError validateConfig(const PIDConfig& candidate);

    static PIDError validateInputs(float setpoint,
                                   float measurement,
                                   float dt,
                                   float feedforward);

    // Calculate the next values without changing controller state.
    PIDError calculateDerivative(float measurement,
                                 float dt,
                                 float& nextDerivative) const;

    PIDError calculateControlOutput(float error,
                                    float derivative,
                                    float dt,
                                    float feedforward,
                                    float& nextIntegral,
                                    float& nextOutput) const;

    // Report an error without resetting controller history.
    PIDResult fail(PIDError error);

public:
    explicit PID(const PIDConfig& config);

    // Validate and initialize the controller.
    bool begin();

    // Calculate the output with optional external feedforward.
    PIDResult compute(float setpoint,
                      float measurement,
                      float dt,
                      float feedforward = 0.0f);

    // Apply valid parameters without requiring another begin().
    bool changeParameters(const PIDConfig& newConfig);

    // Clear the controller's internal state.
    void reset();

    // Read the most recent error.
    PIDError getLastError() const;

    const char* getErrorMessage() const;
};