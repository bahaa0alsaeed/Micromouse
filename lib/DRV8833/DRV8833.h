#pragma once

#include <Arduino.h>

struct DRV8833Config
{
    uint8_t pinIN1;
    uint8_t pinIN2;
    uint8_t pinIN3;
    uint8_t pinIN4;

    uint8_t channelIN1;
    uint8_t channelIN2;
    uint8_t channelIN3;
    uint8_t channelIN4;

    uint32_t freq = 5000;
    uint8_t resolution = 8;
    uint8_t MAX_SPEED = 255;
};

class DRV8833
{
    int rightSpeed = 0;
    int leftSpeed = 0;
    uint8_t MAX_SPEED;
    DRV8833Config config{};

public:
    explicit DRV8833(const DRV8833Config& config);
    void setMotorSpeed(int right, int left);
    void stop();
};