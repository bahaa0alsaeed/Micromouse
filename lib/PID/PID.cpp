
#include "PID.h"
#include <cmath>

// Clamp a value to the configured limits.
static float clampValue(float value, float minimum, float maximum)
{
    if (value > maximum)
        return maximum;

    if (value < minimum)
        return minimum;

    return value;
}

PID::PID(const PIDConfig& config)
{
    this->config = config;
}

bool PID::begin()
{
    configured = false;
    reset();

    // Validate PID gains.
    if (!std::isfinite(config.kp) ||
        !std::isfinite(config.ki) ||
        !std::isfinite(config.kd))
    {
        lastError = PIDError::InvalidGains;
    }

    // Validate the derivative filter parameter.
    else if (!std::isfinite(config.N) ||
             config.N < 0.0f ||
             (config.kd != 0.0f && config.N == 0.0f))
    {
        lastError = PIDError::InvalidFilterParameter;
    }

    // Validate output limits.
    else if (!std::isfinite(config.outputMin) ||
             !std::isfinite(config.outputMax) ||
             config.outputMin >= config.outputMax)
    {
        lastError = PIDError::InvalidOutputLimits;
    }

    // Validate integral limits.
    else if (!std::isfinite(config.integralMin) ||
             !std::isfinite(config.integralMax) ||
             config.integralMin > 0.0f ||
             config.integralMax < 0.0f ||
             config.integralMin > config.integralMax ||
             (config.ki != 0.0f &&
              config.integralMin == config.integralMax))
    {
        lastError = PIDError::InvalidIntegralLimits;
    }

    // Configuration is valid.
    else
    {
        configured = true;
        lastError = PIDError::None;
    }

    return configured;
}

float PID::compute(const float setpoint,
                   const float measurement,
                   float dt,
                   const float feedforward)
{
    // Prevent calculations before successful initialization.
    if (!configured)
    {
        if (lastError == PIDError::None)
            lastError = PIDError::NotInitialized;

        output = 0.0f;
        return output;
    }

    // Validate all inputs before updating the internal state.
    if (!std::isfinite(dt) || dt <= 0.0f)
        return fail(PIDError::InvalidSampleTime);

    if (!std::isfinite(setpoint))
        return fail(PIDError::InvalidSetpoint);

    if (!std::isfinite(measurement))
        return fail(PIDError::InvalidMeasurement);

    if (!std::isfinite(feedforward))
        return fail(PIDError::InvalidFeedforward);

    // Calculate the control error.
    const float error = setpoint - measurement;

    if (!std::isfinite(error))
        return fail(PIDError::NumericalError);

    // Initialize the derivative term to zero.
    float derivative = 0.0f;

    // Skip differentiation on the first sample or after reset().
    if (initialized && config.kd != 0.0f)
    {
        // Calculate the digital derivative filter coefficients.
        const float denominator = 1.0f + config.N * dt;

        if (!std::isfinite(denominator) || denominator <= 0.0f)
            return fail(PIDError::NumericalError);

        const float alpha = 1.0f / denominator;
        const float beta = config.N / denominator;

        // Apply the discrete filtered derivative equation.
        derivative = alpha * previousDerivative +
                     beta * (measurement - previousMeasurement);

        if (!std::isfinite(derivative))
            return fail(PIDError::NumericalError);
    }

    // Calculate proportional, derivative, and feedforward terms.
    // The derivative is subtracted because it is based on measurement.
    const float baseOutput =
        config.kp * error -
        config.kd * derivative +
        feedforward;

    if (!std::isfinite(baseOutput))
        return fail(PIDError::NumericalError);

    // Calculate the candidate integral without updating the state.
    const float integralCandidateRaw = integral + error * dt;

    if (!std::isfinite(integralCandidateRaw))
        return fail(PIDError::NumericalError);

    // Limit the candidate integral.
    const float integralCandidate = clampValue(
        integralCandidateRaw,
        config.integralMin,
        config.integralMax
    );

    // Calculate the expected output with the candidate integral.
    const float candidateOutput =
        baseOutput + config.ki * integralCandidate;

    // Determine how the integral update affects the output.
    const float integralChange =
        config.ki * (integralCandidate - integral);

    if (!std::isfinite(candidateOutput) ||
        !std::isfinite(integralChange))
    {
        return fail(PIDError::NumericalError);
    }

    // Prevent integration from increasing output saturation.
    const bool windupHigh =
        candidateOutput > config.outputMax &&
        integralChange > 0.0f;

    const bool windupLow =
        candidateOutput < config.outputMin &&
        integralChange < 0.0f;

    // Accept integration only if it does not increase windup.
    if (!windupHigh && !windupLow)
        integral = integralCandidate;

    // Calculate the final PID output.
    const float rawOutput =
        baseOutput + config.ki * integral;

    if (!std::isfinite(rawOutput))
        return fail(PIDError::NumericalError);

    // Apply the configured output limits.
    output = clampValue(
        rawOutput,
        config.outputMin,
        config.outputMax
    );

    // Save the current measurement and derivative.
    previousMeasurement = measurement;
    previousDerivative = derivative;

    // Mark the controller as initialized.
    initialized = true;

    // Clear the previous runtime error.
    lastError = PIDError::None;

    return output;
}

void PID::changeParameters(const PIDConfig& newConfig)
{
    // Replace the configuration and require revalidation.
    config = newConfig;
    configured = false;
    lastError = PIDError::NotInitialized;

    reset();
}

void PID::reset()
{
    // Clear the internal controller history.
    previousMeasurement = 0.0f;
    integral = 0.0f;
    previousDerivative = 0.0f;
    output = 0.0f;

    // Prevent differentiation on the next sample.
    initialized = false;

    if (configured)
        lastError = PIDError::None;
}

float PID::fail(PIDError error)
{
    // Discard invalid controller history.
    reset();

    // Store the runtime error.
    lastError = error;

    return output;
}

PIDError PID::getLastError() const
{
    return lastError;
}

const char* PID::getErrorMessage() const
{
    switch (lastError)
    {
        case PIDError::None:
            return "PID: no error.";

        case PIDError::NotInitialized:
            return "PID: call begin() before compute().";

        case PIDError::InvalidGains:
            return "PID: kp, ki, and kd must be finite.";

        case PIDError::InvalidFilterParameter:
            return "PID: N must be finite and positive when kd is nonzero.";

        case PIDError::InvalidOutputLimits:
            return "PID: output limits must be finite with min < max.";

        case PIDError::InvalidIntegralLimits:
            return "PID: invalid integral limits.";

        case PIDError::InvalidSampleTime:
            return "PID: dt must be finite and greater than zero.";

        case PIDError::InvalidSetpoint:
            return "PID: setpoint must be finite.";

        case PIDError::InvalidMeasurement:
            return "PID: measurement must be finite.";

        case PIDError::InvalidFeedforward:
            return "PID: feedforward must be finite.";

        case PIDError::NumericalError:
            return "PID: numerical overflow or invalid calculation.";
    }

    return "PID: unknown error.";
}
