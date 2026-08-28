#include <unity.h>
#include "PID.h"
#include <cmath>

// ============================================================
// Test 01 - Constructor
// ============================================================

void test_constructor()
{
    PIDConfig config;

    config.kp = 2.0f;
    config.ki = 1.0f;
    config.kd = 0.5f;
    config.alpha = 0.7f;

    config.outputMin = -100.0f;
    config.outputMax = 100.0f;

    config.integralMin = -50.0f;
    config.integralMax = 50.0f;

    PID pid(config);

    float output = pid.compute(
        10.0f,
        10.0f,
        0.1f
    );

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        0.0f,
        output
    );
}


// ============================================================
// Test 02 - Proportional term
// ============================================================

void test_proportional()
{
    PIDConfig config;

    config.kp = 2.0f;
    config.ki = 0.0f;
    config.kd = 0.0f;

    config.outputMin = -100.0f;
    config.outputMax = 100.0f;

    PID pid(config);

    float output = pid.compute(
        10.0f,
        8.0f,
        0.1f
    );

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        4.0f,
        output
    );
}


// ============================================================
// Test 03 - Negative proportional error
// ============================================================

void test_negative_error()
{
    PIDConfig config;

    config.kp = 2.0f;
    config.ki = 0.0f;
    config.kd = 0.0f;

    config.outputMin = -100.0f;
    config.outputMax = 100.0f;

    PID pid(config);

    float output = pid.compute(
        8.0f,
        10.0f,
        0.1f
    );

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        -4.0f,
        output
    );
}


// ============================================================
// Test 04 - Zero error
// ============================================================

void test_zero_error()
{
    PIDConfig config;

    config.kp = 2.0f;
    config.ki = 0.0f;
    config.kd = 0.0f;

    config.outputMin = -100.0f;
    config.outputMax = 100.0f;

    PID pid(config);

    float output = pid.compute(
        10.0f,
        10.0f,
        0.1f
    );

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        0.0f,
        output
    );
}


// ============================================================
// Test 05 - Integral accumulation
// ============================================================

void test_integral_accumulation()
{
    PIDConfig config;

    config.kp = 0.0f;
    config.ki = 1.0f;
    config.kd = 0.0f;

    config.outputMin = -100.0f;
    config.outputMax = 100.0f;

    config.integralMin = -100.0f;
    config.integralMax = 100.0f;

    PID pid(config);

    float output1 = pid.compute(
        10.0f,
        8.0f,
        1.0f
    );

    float output2 = pid.compute(
        10.0f,
        8.0f,
        1.0f
    );

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        2.0f,
        output1
    );

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        4.0f,
        output2
    );
}


// ============================================================
// Test 06 - Integral limit
// ============================================================

void test_integral_limit()
{
    PIDConfig config;

    config.kp = 0.0f;
    config.ki = 1.0f;
    config.kd = 0.0f;

    config.outputMin = -100.0f;
    config.outputMax = 100.0f;

    config.integralMin = -5.0f;
    config.integralMax = 5.0f;

    PID pid(config);

    float output = 0.0f;

    for (int i = 0; i < 20; ++i)
    {
        output = pid.compute(
            10.0f,
            0.0f,
            1.0f
        );
    }

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        5.0f,
        output
    );
}


// ============================================================
// Test 07 - Output upper saturation
// ============================================================

void test_output_upper_limit()
{
    PIDConfig config;

    config.kp = 10.0f;
    config.ki = 0.0f;
    config.kd = 0.0f;

    config.outputMin = -10.0f;
    config.outputMax = 10.0f;

    PID pid(config);

    float output = pid.compute(
        10.0f,
        0.0f,
        0.1f
    );

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        10.0f,
        output
    );
}


// ============================================================
// Test 08 - Output lower saturation
// ============================================================

void test_output_lower_limit()
{
    PIDConfig config;

    config.kp = 10.0f;
    config.ki = 0.0f;
    config.kd = 0.0f;

    config.outputMin = -10.0f;
    config.outputMax = 10.0f;

    PID pid(config);

    float output = pid.compute(
        -10.0f,
        0.0f,
        0.1f
    );

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        -10.0f,
        output
    );
}


// ============================================================
// Test 09 - Output inside limits
// ============================================================

void test_output_inside_limits()
{
    PIDConfig config;

    config.kp = 2.0f;
    config.ki = 0.0f;
    config.kd = 0.0f;

    config.outputMin = -10.0f;
    config.outputMax = 10.0f;

    PID pid(config);

    float output = pid.compute(
        2.0f,
        0.0f,
        0.1f
    );

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        4.0f,
        output
    );
}


// ============================================================
// Test 10 - Reset clears integral state
// ============================================================

void test_reset_integral()
{
    PIDConfig config;

    config.kp = 0.0f;
    config.ki = 1.0f;
    config.kd = 0.0f;

    config.outputMin = -100.0f;
    config.outputMax = 100.0f;

    config.integralMin = -100.0f;
    config.integralMax = 100.0f;

    PID pid(config);

    pid.compute(
        10.0f,
        8.0f,
        1.0f
    );

    pid.compute(
        10.0f,
        8.0f,
        1.0f
    );

    pid.reset();

    float output = pid.compute(
        10.0f,
        8.0f,
        1.0f
    );

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        2.0f,
        output
    );
}


// ============================================================
// Test 11 - Reset clears derivative state
// ============================================================

void test_reset_clears_derivative_state()
{
    PIDConfig config;

    config.kp = 0.0f;
    config.ki = 0.0f;
    config.kd = 1.0f;

    config.alpha = 0.7f;

    config.outputMin = -100.0f;
    config.outputMax = 100.0f;

    PID pid(config);

    pid.compute(
        0.0f,
        0.0f,
        1.0f
    );

    pid.compute(
        10.0f,
        0.0f,
        1.0f
    );

    pid.reset();

    float output = pid.compute(
        10.0f,
        0.0f,
        1.0f
    );

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        3.0f,
        output
    );
}


// ============================================================
// Test 12 - Change parameters
// ============================================================

void test_change_parameters()
{
    PIDConfig config;

    config.kp = 1.0f;
    config.ki = 0.0f;
    config.kd = 0.0f;

    config.outputMin = -100.0f;
    config.outputMax = 100.0f;

    PID pid(config);

    float output1 = pid.compute(
        10.0f,
        8.0f,
        0.1f
    );

    PIDConfig newConfig;

    newConfig.kp = 5.0f;
    newConfig.ki = 0.0f;
    newConfig.kd = 0.0f;

    newConfig.outputMin = -100.0f;
    newConfig.outputMax = 100.0f;

    pid.changeParameters(newConfig);

    float output2 = pid.compute(
        10.0f,
        8.0f,
        0.1f
    );

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        2.0f,
        output1
    );

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        10.0f,
        output2
    );
}


// ============================================================
// Test 13 - Derivative response
// ============================================================

void test_derivative_response()
{
    PIDConfig config;

    config.kp = 0.0f;
    config.ki = 0.0f;
    config.kd = 1.0f;

    config.alpha = 0.0f;

    config.outputMin = -100.0f;
    config.outputMax = 100.0f;

    PID pid(config);

    pid.compute(
        0.0f,
        0.0f,
        1.0f
    );

    float output = pid.compute(
        10.0f,
        0.0f,
        1.0f
    );

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        10.0f,
        output
    );
}


// ============================================================
// Test 14 - Zero dt does not produce invalid result
// ============================================================


void test_zero_dt()
{
    PIDConfig config;

    config.kp = 2.0f;
    config.ki = 1.0f;
    config.kd = 1.0f;

    config.outputMin = -100.0f;
    config.outputMax = 100.0f;

    config.integralMin = -100.0f;
    config.integralMax = 100.0f;

    PID pid(config);

    float output = pid.compute(
        10.0f,
        10.0f,
        0.0f
    );

    TEST_ASSERT_TRUE(std::isfinite(output));
}


// ============================================================
// Test 15 - Different dt changes integral contribution
// ============================================================

void test_dt_affects_integral()
{
    PIDConfig config;

    config.kp = 0.0f;
    config.ki = 1.0f;
    config.kd = 0.0f;

    config.outputMin = -100.0f;
    config.outputMax = 100.0f;

    config.integralMin = -100.0f;
    config.integralMax = 100.0f;

    PID pid(config);

    float output = pid.compute(
        10.0f,
        8.0f,
        0.5f
    );

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        1.0f,
        output
    );
}


// ============================================================
// Test 16 - Combined PID
// ============================================================

void test_combined_pi()
{
    PIDConfig config;

    config.kp = 2.0f;
    config.ki = 1.0f;
    config.kd = 0.0f;

    config.outputMin = -100.0f;
    config.outputMax = 100.0f;

    config.integralMin = -100.0f;
    config.integralMax = 100.0f;

    PID pid(config);

    float output = pid.compute(
        10.0f,
        8.0f,
        1.0f
    );

    // P = 2 * 2 = 4
    // I = 2 * 1 = 2
    // Total = 6

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        6.0f,
        output
    );
}


// ============================================================
// Test 17 - Output remains bounded after repeated error
// ============================================================

void test_output_remains_bounded()
{
    PIDConfig config;

    config.kp = 10.0f;
    config.ki = 5.0f;
    config.kd = 1.0f;

    config.outputMin = -20.0f;
    config.outputMax = 20.0f;

    config.integralMin = -100.0f;
    config.integralMax = 100.0f;

    PID pid(config);

    for (int i = 0; i < 100; ++i)
    {
        float output = pid.compute(
            100.0f,
            0.0f,
            0.01f
        );

        TEST_ASSERT_TRUE(output <= 20.0f);
        TEST_ASSERT_TRUE(output >= -20.0f);
    }
}


// ============================================================
// Test 18 - Negative integral limit
// ============================================================

void test_negative_integral_limit()
{
    PIDConfig config;

    config.kp = 0.0f;
    config.ki = 1.0f;
    config.kd = 0.0f;

    config.outputMin = -100.0f;
    config.outputMax = 100.0f;

    config.integralMin = -5.0f;
    config.integralMax = 5.0f;

    PID pid(config);

    float output = 0.0f;

    for (int i = 0; i < 20; ++i)
    {
        output = pid.compute(
            0.0f,
            10.0f,
            1.0f
        );
    }

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        -5.0f,
        output
    );
}


// ============================================================
// Test 19 - Reset returns PID to initial state
// ============================================================

void test_reset_returns_to_initial_state()
{
    PIDConfig config;

    config.kp = 2.0f;
    config.ki = 1.0f;
    config.kd = 0.0f;

    config.outputMin = -100.0f;
    config.outputMax = 100.0f;

    config.integralMin = -100.0f;
    config.integralMax = 100.0f;

    PID pid(config);

    float first = pid.compute(
        10.0f,
        8.0f,
        1.0f
    );

    pid.compute(
        10.0f,
        8.0f,
        1.0f
    );

    pid.reset();

    float afterReset = pid.compute(
        10.0f,
        8.0f,
        1.0f
    );

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        first,
        afterReset
    );
}


// ============================================================
// Test 20 - Output limit remains valid after parameter change
// ============================================================

void test_change_parameters_updates_limits()
{
    PIDConfig config;

    config.kp = 10.0f;
    config.ki = 0.0f;
    config.kd = 0.0f;

    config.outputMin = -100.0f;
    config.outputMax = 100.0f;

    PID pid(config);

    PIDConfig newConfig;

    newConfig.kp = 10.0f;
    newConfig.ki = 0.0f;
    newConfig.kd = 0.0f;

    newConfig.outputMin = -5.0f;
    newConfig.outputMax = 5.0f;

    pid.changeParameters(newConfig);

    float output = pid.compute(
        10.0f,
        0.0f,
        0.1f
    );

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        5.0f,
        output
    );
}

// ============================================================
// Test 21 - Derivative low-pass filter
// ============================================================

void test_derivative_filter()
{
    PIDConfig config;

    config.kp = 0.0f;
    config.ki = 0.0f;
    config.kd = 1.0f;

    config.alpha = 0.5f;

    config.outputMin = -100.0f;
    config.outputMax = 100.0f;

    PID pid(config);

    pid.compute(
        0.0f,
        0.0f,
        1.0f
    );

    float output = pid.compute(
        10.0f,
        0.0f,
        1.0f
    );

    // raw derivative = 10
    // filtered derivative = 0.5 * 0 + 0.5 * 10
    //                      = 5

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        5.0f,
        output
    );
}


// ============================================================
// Test 22 - Derivative filter with alpha = 1
// ============================================================

void test_derivative_filter_alpha_one()
{
    PIDConfig config;

    config.kp = 0.0f;
    config.ki = 0.0f;
    config.kd = 1.0f;

    config.alpha = 1.0f;

    config.outputMin = -100.0f;
    config.outputMax = 100.0f;

    PID pid(config);

    pid.compute(
        0.0f,
        0.0f,
        1.0f
    );

    float output = pid.compute(
        10.0f,
        0.0f,
        1.0f
    );

    // derivative =
    // 1.0 * previousDerivative +
    // 0.0 * rawDerivative
    //
    // previousDerivative = 0

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        0.0f,
        output
    );
}


// ============================================================
// Test 23 - Derivative filter with alpha = 0
// ============================================================

void test_derivative_filter_alpha_zero()
{
    PIDConfig config;

    config.kp = 0.0f;
    config.ki = 0.0f;
    config.kd = 1.0f;

    config.alpha = 0.0f;

    config.outputMin = -100.0f;
    config.outputMax = 100.0f;

    PID pid(config);

    pid.compute(
        0.0f,
        0.0f,
        1.0f
    );

    float output = pid.compute(
        10.0f,
        0.0f,
        1.0f
    );

    // alpha = 0 means no filtering
    // derivative = raw derivative = 10

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        10.0f,
        output
    );
}


// ============================================================
// Test 24 - Change parameters preserves integral state
// ============================================================

void test_change_parameters_preserves_state()
{
    PIDConfig config;

    config.kp = 0.0f;
    config.ki = 1.0f;
    config.kd = 0.0f;

    config.outputMin = -100.0f;
    config.outputMax = 100.0f;

    config.integralMin = -100.0f;
    config.integralMax = 100.0f;

    PID pid(config);

    pid.compute(
        10.0f,
        0.0f,
        1.0f
    );

    PIDConfig newConfig;

    newConfig.kp = 1.0f;
    newConfig.ki = 1.0f;
    newConfig.kd = 0.0f;

    newConfig.outputMin = -100.0f;
    newConfig.outputMax = 100.0f;

    newConfig.integralMin = -100.0f;
    newConfig.integralMax = 100.0f;

    pid.changeParameters(newConfig);

    float output = pid.compute(
        10.0f,
        0.0f,
        1.0f
    );

    // Integral before change = 10
    //
    // Second compute:
    // Integral = 10 + 10 = 20
    // P = 1 * 10 = 10
    // I = 1 * 20 = 20
    //
    // Total = 30

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        30.0f,
        output
    );
}


// ============================================================
// Test 25 - Change parameters updates integral limits
// ============================================================

void test_change_parameters_updates_integral_limits()
{
    PIDConfig config;

    config.kp = 0.0f;
    config.ki = 1.0f;
    config.kd = 0.0f;

    config.outputMin = -100.0f;
    config.outputMax = 100.0f;

    config.integralMin = -100.0f;
    config.integralMax = 100.0f;

    PID pid(config);

    PIDConfig newConfig;

    newConfig.kp = 0.0f;
    newConfig.ki = 1.0f;
    newConfig.kd = 0.0f;

    newConfig.outputMin = -100.0f;
    newConfig.outputMax = 100.0f;

    newConfig.integralMin = -5.0f;
    newConfig.integralMax = 5.0f;

    pid.changeParameters(newConfig);

    float output = pid.compute(
        10.0f,
        0.0f,
        1.0f
    );

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        5.0f,
        output
    );
}


// ============================================================
// Test 26 - Change parameters updates alpha
// ============================================================

void test_change_parameters_updates_alpha()
{
    PIDConfig config;

    config.kp = 0.0f;
    config.ki = 0.0f;
    config.kd = 1.0f;

    config.alpha = 0.0f;

    config.outputMin = -100.0f;
    config.outputMax = 100.0f;

    PID pid(config);

    pid.compute(
        0.0f,
        0.0f,
        1.0f
    );

    PIDConfig newConfig;

    newConfig.kp = 0.0f;
    newConfig.ki = 0.0f;
    newConfig.kd = 1.0f;

    newConfig.alpha = 0.5f;

    newConfig.outputMin = -100.0f;
    newConfig.outputMax = 100.0f;

    pid.changeParameters(newConfig);

    float output = pid.compute(
        10.0f,
        0.0f,
        1.0f
    );

    // alpha = 0.5
    // derivative = 0.5 * 0 + 0.5 * 10
    //             = 5

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        5.0f,
        output
    );
}


// ============================================================
// Test 27 - Reset clears all PID state
// ============================================================

void test_reset_clears_all_state()
{
    PIDConfig config;

    config.kp = 1.0f;
    config.ki = 1.0f;
    config.kd = 1.0f;

    config.alpha = 0.0f;

    config.outputMin = -100.0f;
    config.outputMax = 100.0f;

    config.integralMin = -100.0f;
    config.integralMax = 100.0f;

    PID pid(config);

    pid.compute(
        10.0f,
        0.0f,
        1.0f
    );

    pid.compute(
        20.0f,
        0.0f,
        1.0f
    );

    pid.reset();

    float output = pid.compute(
        10.0f,
        0.0f,
        1.0f
    );

    // After reset:
    //
    // error = 10
    // integral = 10
    // derivative = 10
    //
    // P = 10
    // I = 10
    // D = 10
    //
    // Total = 30

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        30.0f,
        output
    );
}


// ============================================================
// Test 28 - Negative dt does not produce invalid result
// ============================================================

void test_negative_dt()
{
    PIDConfig config;

    config.kp = 2.0f;
    config.ki = 1.0f;
    config.kd = 1.0f;

    config.alpha = 0.7f;

    config.outputMin = -100.0f;
    config.outputMax = 100.0f;

    config.integralMin = -100.0f;
    config.integralMax = 100.0f;

    PID pid(config);

    float output = pid.compute(
        10.0f,
        10.0f,
        -1.0f
    );

    TEST_ASSERT_TRUE(std::isfinite(output));
}


// ============================================================
// Test 29 - Derivative responds to decreasing error
// ============================================================

void test_derivative_negative_change()
{
    PIDConfig config;

    config.kp = 0.0f;
    config.ki = 0.0f;
    config.kd = 1.0f;

    config.alpha = 0.0f;

    config.outputMin = -100.0f;
    config.outputMax = 100.0f;

    PID pid(config);

    pid.compute(
        10.0f,
        0.0f,
        1.0f
    );

    float output = pid.compute(
        0.0f,
        0.0f,
        1.0f
    );

    // error changed from 10 to 0
    // derivative = (0 - 10) / 1
    //             = -10

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        -10.0f,
        output
    );
}


// ============================================================
// Test 30 - Derivative responds to measurement change
// ============================================================

void test_derivative_measurement_change()
{
    PIDConfig config;

    config.kp = 0.0f;
    config.ki = 0.0f;
    config.kd = 1.0f;

    config.alpha = 0.0f;

    config.outputMin = -100.0f;
    config.outputMax = 100.0f;

    PID pid(config);

    pid.compute(
        10.0f,
        0.0f,
        1.0f
    );

    float output = pid.compute(
        10.0f,
        5.0f,
        1.0f
    );

    // error changed from 10 to 5
    // derivative = (5 - 10) / 1
    //             = -5

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        -5.0f,
        output
    );
}


// ============================================================
// Test 31 - Integral stops at upper limit
// ============================================================

void test_integral_stays_at_upper_limit()
{
    PIDConfig config;

    config.kp = 0.0f;
    config.ki = 1.0f;
    config.kd = 0.0f;

    config.outputMin = -100.0f;
    config.outputMax = 100.0f;

    config.integralMin = -5.0f;
    config.integralMax = 5.0f;

    PID pid(config);

    float output1 = pid.compute(
        10.0f,
        0.0f,
        1.0f
    );

    float output2 = pid.compute(
        10.0f,
        0.0f,
        1.0f
    );

    float output3 = pid.compute(
        10.0f,
        0.0f,
        1.0f
    );

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        5.0f,
        output1
    );

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        5.0f,
        output2
    );

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        5.0f,
        output3
    );
}


// ============================================================
// Test 32 - Integral stops at lower limit
// ============================================================

void test_integral_stays_at_lower_limit()
{
    PIDConfig config;

    config.kp = 0.0f;
    config.ki = 1.0f;
    config.kd = 0.0f;

    config.outputMin = -100.0f;
    config.outputMax = 100.0f;

    config.integralMin = -5.0f;
    config.integralMax = 5.0f;

    PID pid(config);

    float output1 = pid.compute(
        0.0f,
        10.0f,
        1.0f
    );

    float output2 = pid.compute(
        0.0f,
        10.0f,
        1.0f
    );

    float output3 = pid.compute(
        0.0f,
        10.0f,
        1.0f
    );

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        -5.0f,
        output1
    );

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        -5.0f,
        output2
    );

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        -5.0f,
        output3
    );
}


// ============================================================
// Test 33 - Output remains finite during normal operation
// ============================================================

void test_output_remains_finite()
{
    PIDConfig config;

    config.kp = 2.0f;
    config.ki = 1.0f;
    config.kd = 1.0f;

    config.alpha = 0.5f;

    config.outputMin = -100.0f;
    config.outputMax = 100.0f;

    config.integralMin = -50.0f;
    config.integralMax = 50.0f;

    PID pid(config);

    for (int i = 0; i < 1000; ++i)
    {
        float output = pid.compute(
            10.0f,
            static_cast<float>(i) * 0.01f,
            0.01f
        );

        TEST_ASSERT_TRUE(std::isfinite(output));
    }
}


// ============================================================
// Main
// ============================================================

int main()
{
    UNITY_BEGIN();

    RUN_TEST(test_constructor);
    RUN_TEST(test_proportional);
    RUN_TEST(test_negative_error);
    RUN_TEST(test_zero_error);
    RUN_TEST(test_integral_accumulation);
    RUN_TEST(test_integral_limit);
    RUN_TEST(test_output_upper_limit);
    RUN_TEST(test_output_lower_limit);
    RUN_TEST(test_output_inside_limits);
    RUN_TEST(test_reset_integral);
    RUN_TEST(test_reset_clears_derivative_state);
    RUN_TEST(test_change_parameters);
    RUN_TEST(test_derivative_response);
    RUN_TEST(test_zero_dt);
    RUN_TEST(test_dt_affects_integral);
    RUN_TEST(test_combined_pi);
    RUN_TEST(test_output_remains_bounded);
    RUN_TEST(test_negative_integral_limit);
    RUN_TEST(test_reset_returns_to_initial_state);
    RUN_TEST(test_change_parameters_updates_limits);
    RUN_TEST(test_derivative_filter);
    RUN_TEST(test_derivative_filter_alpha_one);
    RUN_TEST(test_derivative_filter_alpha_zero);
    RUN_TEST(test_change_parameters_preserves_state);
    RUN_TEST(test_change_parameters_updates_integral_limits);
    RUN_TEST(test_change_parameters_updates_alpha);
    RUN_TEST(test_reset_clears_all_state);
    RUN_TEST(test_negative_dt);
    RUN_TEST(test_derivative_negative_change);
    RUN_TEST(test_derivative_measurement_change);
    RUN_TEST(test_integral_stays_at_upper_limit);
    RUN_TEST(test_integral_stays_at_lower_limit);
    RUN_TEST(test_output_remains_finite);

    return UNITY_END();
}