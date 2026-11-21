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

// ------------------------------------------------------------
// Configuration Validation
// ------------------------------------------------------------

PIDError PID::validateConfig(const PIDConfig& candidate)
{
    if (!std::isfinite(candidate.kp) ||
        !std::isfinite(candidate.ki) ||
        !std::isfinite(candidate.kd))
    {
        return PIDError::InvalidGains;
    }

    if (!std::isfinite(candidate.N) ||
        candidate.N < 0.0f ||
        (candidate.kd != 0.0f && candidate.N == 0.0f))
    {
        return PIDError::InvalidFilterParameter;
    }

    if (!std::isfinite(candidate.outputMin) ||
        !std::isfinite(candidate.outputMax) ||
        candidate.outputMin >= candidate.outputMax)
    {
        return PIDError::InvalidOutputLimits;
    }

    if (!std::isfinite(candidate.integralMin) ||
        !std::isfinite(candidate.integralMax) ||
        candidate.integralMin > 0.0f ||
        candidate.integralMax < 0.0f ||
        candidate.integralMin > candidate.integralMax ||
        (candidate.ki != 0.0f &&
         candidate.integralMin == candidate.integralMax))
    {
        return PIDError::InvalidIntegralLimits;
    }

    return PIDError::None;
}

// ------------------------------------------------------------
// Input Validation
// ------------------------------------------------------------

PIDError PID::validateInputs(float setpoint,
                             float measurement,
                             float dt,
                             float feedforward)
{
    if (!std::isfinite(dt) || dt <= 0.0f)
        return PIDError::InvalidSampleTime;

    if (!std::isfinite(setpoint))
        return PIDError::InvalidSetpoint;

    if (!std::isfinite(measurement))
        return PIDError::InvalidMeasurement;

    if (!std::isfinite(feedforward))
        return PIDError::InvalidFeedforward;

    return PIDError::None;
}

// ------------------------------------------------------------
// Derivative Calculation
// ------------------------------------------------------------

PIDError PID::calculateDerivative(float measurement,
                                  float dt,
                                  float& nextDerivative) const
{
    // Skip differentiation on the first successful sample.
    nextDerivative = 0.0f;

    if (!initialized || config.kd == 0.0f)
        return PIDError::None;

    const float denominator = 1.0f + config.N * dt;

    if (!std::isfinite(denominator) || denominator <= 0.0f)
        return PIDError::NumericalError;

    const float alpha = 1.0f / denominator;
    const float beta = config.N / denominator;

    // Filtered derivative of measurement, not of error.
    nextDerivative =
        alpha * previousDerivative +
        beta * (measurement - previousMeasurement);

    if (!std::isfinite(nextDerivative))
        return PIDError::NumericalError;

    return PIDError::None;
}

// ------------------------------------------------------------
// Output and Anti-Windup Calculation
// ------------------------------------------------------------

PIDError PID::calculateControlOutput(float error,
                                     float derivative,
                                     float dt,
                                     float feedforward,
                                     float& nextIntegral,
                                     float& nextOutput) const
{
    // Compute P, D and feedforward before adding the integral term.
    const float baseOutput =
        config.kp * error -
        config.kd * derivative +
        feedforward;

    if (!std::isfinite(baseOutput))
        return PIDError::NumericalError;

    // Propose an integral update without saving it.
    const float rawIntegral = integral + error * dt;

    if (!std::isfinite(rawIntegral))
        return PIDError::NumericalError;

    const float candidateIntegral = clampValue(
        rawIntegral,
        config.integralMin,
        config.integralMax
    );

    const float candidateOutput =
        baseOutput + config.ki * candidateIntegral;

    const float integralChange =
        config.ki * (candidateIntegral - integral);

    if (!std::isfinite(candidateOutput) ||
        !std::isfinite(integralChange))
    {
        return PIDError::NumericalError;
    }

    // Do not integrate further into output saturation.
    const bool windupHigh =
        candidateOutput > config.outputMax &&
        integralChange > 0.0f;

    const bool windupLow =
        candidateOutput < config.outputMin &&
        integralChange < 0.0f;

    nextIntegral = (windupHigh || windupLow)
        ? integral
        : candidateIntegral;

    // Calculate the final output.
    const float rawOutput =
        baseOutput + config.ki * nextIntegral;

    if (!std::isfinite(rawOutput))
        return PIDError::NumericalError;

    nextOutput = clampValue(
        rawOutput,
        config.outputMin,
        config.outputMax
    );

    return PIDError::None;
}

// ------------------------------------------------------------
// Controller Initialization
// ------------------------------------------------------------

bool PID::begin()
{
    configured = false;
    reset();

    lastError = validateConfig(config);

    configured = (lastError == PIDError::None);

    return configured;
}

// ------------------------------------------------------------
// PID Computation
// ------------------------------------------------------------

PIDResult PID::compute(float setpoint,
                       float measurement,
                       float dt,
                       float feedforward)
{
    if (!configured)
    {
        const PIDError error = (lastError == PIDError::None)
            ? PIDError::NotInitialized
            : lastError;

        return fail(error);
    }

    // Step 1: Validate inputs.
    const PIDError inputError = validateInputs(
        setpoint, measurement, dt, feedforward
    );

    if (inputError != PIDError::None)
        return fail(inputError);

    // Step 2: Calculate the control error.
    const float error = setpoint - measurement;

    if (!std::isfinite(error))
        return fail(PIDError::NumericalError);

    // Step 3: Calculate the filtered derivative.
    float nextDerivative = 0.0f;

    const PIDError derivativeError = calculateDerivative(
        measurement, dt, nextDerivative
    );

    if (derivativeError != PIDError::None)
        return fail(derivativeError);

    // Step 4: Calculate output and integral.
    float nextIntegral = integral;
    float nextOutput = output;

    const PIDError controlError = calculateControlOutput(
        error,
        nextDerivative,
        dt,
        feedforward,
        nextIntegral,
        nextOutput
    );

    if (controlError != PIDError::None)
        return fail(controlError);

    // Step 5: Save the new state only after all steps succeed.
    integral = nextIntegral;
    output = nextOutput;

    previousMeasurement = measurement;
    previousDerivative = nextDerivative;

    initialized = true;
    lastError = PIDError::None;

    return PIDResult{
        output,
        true,
        PIDError::None
    };
}

// ------------------------------------------------------------
// Parameter Updates
// ------------------------------------------------------------

bool PID::changeParameters(const PIDConfig& newConfig)
{
    const PIDError validationError = validateConfig(newConfig);

    // Reject invalid parameters and preserve controller state.
    if (validationError != PIDError::None)
    {
        lastError = validationError;
        return false;
    }

    config = newConfig;
    configured = true;

    // Clear history when applying a new configuration.
    reset();

    return true;
}

// ------------------------------------------------------------
// State Reset and Error Handling
// ------------------------------------------------------------

void PID::reset()
{
    previousMeasurement = 0.0f;
    integral = 0.0f;
    previousDerivative = 0.0f;
    output = 0.0f;

    initialized = false;

    if (configured)
        lastError = PIDError::None;
}

PIDResult PID::fail(PIDError error)
{
    // Preserve the last valid output and control history.
    lastError = error;

    return PIDResult{
        output,
        false,
        error
    };
}

// ------------------------------------------------------------
// Error Reporting
// ------------------------------------------------------------

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