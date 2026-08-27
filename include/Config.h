#pragma once
#include "DRV8833.h"
#include "Encoder.h"
#include "WallSensors.h"
#include "Navigation.h"
#include "PID.h"

namespace Config
{
    // ============================================================
    // Motor Driver
    // ============================================================

    inline constexpr DRV8833Config driver{
        .IN1 = 5,
        .IN2 = 6,
        .IN3 = 9,
        .IN4 = 10,
        .MAX_SPEED = 255
    };

    // ============================================================
    // Encoders
    // ============================================================

    inline constexpr EncoderConfig leftEncoder{
        .pin = 2,
        .ticksPerRevolution = 360,
        .wheelDiameter = 34.0f,
        .gearRatio = 1.0f
    };

    inline constexpr EncoderConfig rightEncoder{
        .pin = 3,
        .ticksPerRevolution = 360,
        .wheelDiameter = 34.0f,
        .gearRatio = 1.0f
    };

    // ============================================================
    // Wall Sensors
    // ============================================================

    inline constexpr WallSensorConfig wallSensors{
        .SDA = 20,
        .SCL = 21,

        .leftXshutPin = 4,
        .frontXshutPin = 7,
        .rightXshutPin = 8,

        .frontThreshold = 100.0f,
        .leftThreshold = 80.0f,
        .rightThreshold = 80.0f
    };

    // ============================================================
    // Navigation
    // ============================================================

    inline constexpr NavigationConfig navigation{
        .wheelBase = 90.0f,
        .cellSize = 180.0f
    };

    // ============================================================
    // PID Controllers
    // ============================================================

    inline constexpr PIDConfig positionPID{
        .kp = 1.0f,
        .ki = 0.0f,
        .kd = 0.0f,

        .outputMin = -100.0f,
        .outputMax = 100.0f,

        .integralMin = -50.0f,
        .integralMax = 50.0f
    };

    inline constexpr PIDConfig leftSpeedPID{
        .kp = 1.0f,
        .ki = 0.0f,
        .kd = 0.0f,

        .outputMin = -255.0f,
        .outputMax = 255.0f,

        .integralMin = -100.0f,
        .integralMax = 100.0f
    };

    inline constexpr PIDConfig rightSpeedPID{
        .kp = 1.0f,
        .ki = 0.0f,
        .kd = 0.0f,

        .outputMin = -255.0f,
        .outputMax = 255.0f,

        .integralMin = -100.0f,
        .integralMax = 100.0f
    };
}
