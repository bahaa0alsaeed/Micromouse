#pragma once
#include "VL53L0X.h"

struct WallSensorConfig
{
    unsigned int SDA = 0;
    unsigned int SCL = 0;
    unsigned int leftXshutPin = 0;
    unsigned int frontXshutPin = 0;
    unsigned int rightXshutPin = 0;
    float frontThreshold = 0.0;
    float leftThreshold = 0.0;
    float rightThreshold = 0.0;
};

struct WallReadings
{
    float front = 0.0;
    float left = 0.0;
    float right = 0.0;
};

class WallSensors
{
    VL53L0X frontSensor;
    VL53L0X leftSensor;
    VL53L0X rightSensor;
    WallSensorConfig config;
    WallReadings readings;

public:
    explicit WallSensors(const WallSensorConfig& config);
    virtual ~WallSensors() = default;

    WallReadings getReadings();
    virtual bool wallFront();
    virtual bool wallRight();
    virtual bool wallLeft();
};

