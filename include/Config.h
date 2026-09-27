#pragma once

#include "DRV8833.h"
#include "Encoder.h"
#include "WallSensors.h"
#include "Navigation.h"
#include "PID.h"

namespace Config
{
    // ============================================================
    // Raw Configuration Values
    // ============================================================

    // ---------------- DRV8833 ----------------
    inline constexpr uint8_t pinIN1 = 5;
    inline constexpr uint8_t pinIN2 = 6;
    inline constexpr uint8_t pinIN3 = 10;
    inline constexpr uint8_t pinIN4 = 1;

    inline constexpr uint8_t channelIN1 = 0;
    inline constexpr uint8_t channelIN2 = 1;
    inline constexpr uint8_t channelIN3 = 2;
    inline constexpr uint8_t channelIN4 = 3;

    inline constexpr uint32_t freq = 5000;
    inline constexpr uint8_t resolution = 8;
    inline constexpr uint8_t MAX_SPEED = 255;

    // ---------------- Encoder ----------------
    inline constexpr uint8_t LEFT_ENCODER_PIN = 2;
    inline constexpr uint8_t RIGHT_ENCODER_PIN = 3;
    inline constexpr int TICKS_PER_REVOLUTION = 360;
    inline constexpr float WHEEL_DIAMETER = 34.0f;
    inline constexpr float GEAR_RATIO = 1.0f;

    // ---------------- WallSensors ----------------
    inline constexpr uint8_t FRONT_SDA = 20;
    inline constexpr uint8_t FRONT_SCL = 21;

    inline constexpr uint8_t LEFT_SDA = 20;
    inline constexpr uint8_t LEFT_SCL = 21;

    inline constexpr uint8_t RIGHT_SDA = 20;
    inline constexpr uint8_t RIGHT_SCL = 21;

    inline constexpr uint8_t LEFT_XSHUT_PIN = 4;
    inline constexpr uint8_t FRONT_XSHUT_PIN = 7;
    inline constexpr uint8_t RIGHT_XSHUT_PIN = 8;

    inline constexpr float FRONT_THRESHOLD = 100.0f;
    inline constexpr float LEFT_THRESHOLD = 80.0f;
    inline constexpr float RIGHT_THRESHOLD = 80.0f;


    // ---------------- Navigation ----------------
    inline constexpr float WHEEL_BASE = 90.0f;
    inline constexpr float CELL_SIZE = 180.0f;

    // ---------------- PID - Position Parameters ----------------
    inline constexpr float POSITION_KP = 1.0f;
    inline constexpr float POSITION_KI = 0.0f;
    inline constexpr float POSITION_KD = 0.0f;

    inline constexpr float POSITION_OUTPUT_MIN = -100.0f;
    inline constexpr float POSITION_OUTPUT_MAX = 100.0f;
    inline constexpr float POSITION_INTEGRAL_MIN = -50.0f;
    inline constexpr float POSITION_INTEGRAL_MAX = 50.0f;

    // ---------------- PID - Speed Parameters ----------------
    inline constexpr float LEFT_SPEED_KP = 1.0f;
    inline constexpr float LEFT_SPEED_KI = 0.0f;
    inline constexpr float LEFT_SPEED_KD = 0.0f;

    inline constexpr float RIGHT_SPEED_KP = 1.0f;
    inline constexpr float RIGHT_SPEED_KI = 0.0f;
    inline constexpr float RIGHT_SPEED_KD = 0.0f;

    inline constexpr float SPEED_OUTPUT_MIN = -255.0f;
    inline constexpr float SPEED_OUTPUT_MAX = 255.0f;
    inline constexpr float SPEED_INTEGRAL_MIN = -100.0f;
    inline constexpr float SPEED_INTEGRAL_MAX = 100.0f;





    // ============================================================
    // Component Struct Configurations
    // ============================================================

    // Motor Driver
    inline constexpr DRV8833Config driver{
        .pinIN1 = pinIN1,
        .pinIN2 = pinIN2,
        .pinIN3 = pinIN3,
        .pinIN4 = pinIN4,

        .channelIN1 = channelIN1,
        .channelIN2 = channelIN2,
        .channelIN3 = channelIN3,
        .channelIN4 = channelIN4,

        .freq = freq,
        .resolution = resolution,
        .MAX_SPEED = MAX_SPEED
    };

    // Encoders
    inline constexpr EncoderConfig leftEncoder{
        .pin = LEFT_ENCODER_PIN,
        .ticksPerRevolution = TICKS_PER_REVOLUTION,
        .wheelDiameter = WHEEL_DIAMETER,
        .gearRatio = GEAR_RATIO
    };

    inline constexpr EncoderConfig rightEncoder{
        .pin = RIGHT_ENCODER_PIN,
        .ticksPerRevolution = TICKS_PER_REVOLUTION,
        .wheelDiameter = WHEEL_DIAMETER,
        .gearRatio = GEAR_RATIO
    };

    // Wall Sensors
    inline constexpr WallSensorConfig wallSensors{
        .frontBus = { .SDA = FRONT_SDA, .SCL = FRONT_SCL },
        .leftBus = { .SDA = LEFT_SDA, .SCL = LEFT_SCL },
        .rightBus = { .SDA = RIGHT_SDA, .SCL = RIGHT_SCL },

        .leftXshutPin = LEFT_XSHUT_PIN,
        .frontXshutPin = FRONT_XSHUT_PIN,
        .rightXshutPin = RIGHT_XSHUT_PIN,
        .frontThreshold = FRONT_THRESHOLD,
        .leftThreshold = LEFT_THRESHOLD,
        .rightThreshold = RIGHT_THRESHOLD
    };

    // Navigation
    inline constexpr NavigationConfig navigation{
        .wheelBase = WHEEL_BASE,
        .cellSize = CELL_SIZE
    };

    // PID Configurations
    inline constexpr PIDConfig positionPID{
        .kp = POSITION_KP,
        .ki = POSITION_KI,
        .kd = POSITION_KD,
        .outputMin = POSITION_OUTPUT_MIN,
        .outputMax = POSITION_OUTPUT_MAX,
        .integralMin = POSITION_INTEGRAL_MIN,
        .integralMax = POSITION_INTEGRAL_MAX
    };

    inline constexpr PIDConfig leftSpeedPID{
        .kp = LEFT_SPEED_KP,
        .ki = LEFT_SPEED_KI,
        .kd = LEFT_SPEED_KD,
        .outputMin = SPEED_OUTPUT_MIN,
        .outputMax = SPEED_OUTPUT_MAX,
        .integralMin = SPEED_INTEGRAL_MIN,
        .integralMax = SPEED_INTEGRAL_MAX
    };

    inline constexpr PIDConfig rightSpeedPID{
        .kp = RIGHT_SPEED_KP,
        .ki = RIGHT_SPEED_KI,
        .kd = RIGHT_SPEED_KD,
        .outputMin = SPEED_OUTPUT_MIN,
        .outputMax = SPEED_OUTPUT_MAX,
        .integralMin = SPEED_INTEGRAL_MIN,
        .integralMax = SPEED_INTEGRAL_MAX
    };
}