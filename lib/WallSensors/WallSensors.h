#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <VL53L0X.h>
#include "IWallSensors.h"

struct WallSensorBusConfig
{
    uint8_t SDA = 0;
    uint8_t SCL = 0;
};


struct WallSensorConfig
{
    WallSensorBusConfig frontBus;
    WallSensorBusConfig leftBus;
    WallSensorBusConfig rightBus;

    uint8_t leftXshutPin = 0;
    uint8_t frontXshutPin = 0;
    uint8_t rightXshutPin = 0;

    float frontThreshold = 0.0f;
    float leftThreshold = 0.0f;
    float rightThreshold = 0.0f;
};


struct WallReadings
{
    float front = 0.0f;
    float left = 0.0f;
    float right = 0.0f;
};


class WallSensors : public IWallSensors
{
    static constexpr uint8_t DEFAULT_ADDRESS = 0x29;

    static constexpr uint8_t FRONT_ADDRESS = 0x30;
    static constexpr uint8_t LEFT_ADDRESS = 0x31;
    static constexpr uint8_t RIGHT_ADDRESS = 0x32;

    static constexpr uint8_t MAX_BUSES = 2;

    struct Bus
    {
        TwoWire* wire = nullptr;
        uint8_t SDA = 0;
        uint8_t SCL = 0;
    };

    VL53L0X frontSensor;
    VL53L0X leftSensor;
    VL53L0X rightSensor;

    WallSensorConfig config;
    WallReadings readings;

    Bus buses[MAX_BUSES];
    uint8_t busCount = 0;

    bool initialized = false;

    TwoWire* getBus(const WallSensorBusConfig& busConfig) const;
    TwoWire* createBus(const WallSensorBusConfig& busConfig);

    bool initializeSensor(VL53L0X& sensor, TwoWire& wire, uint8_t xshutPin, uint8_t address);

    static bool isReady(VL53L0X& sensor);
    static bool readSensor(VL53L0X& sensor, float& reading);

public:
    explicit WallSensors(const WallSensorConfig& config);

    bool begin();

    void update() override;

    WallReadings getReadings() const;

    bool wallFront() override;
    bool wallRight() override;
    bool wallLeft() override;
};

