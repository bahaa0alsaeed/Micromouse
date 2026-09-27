#include "DRV8833.h"
#include <Arduino.h>
#include <cmath>

DRV8833::DRV8833(const DRV8833Config& config)
{
    DRV8833::config = config;
    MAX_SPEED = config.MAX_SPEED;

    pinMode(config.pinIN1, OUTPUT);
    pinMode(config.pinIN2, OUTPUT);
    pinMode(config.pinIN3, OUTPUT);
    pinMode(config.pinIN4, OUTPUT);

    ledcSetup(config.channelIN1, config.freq, config.resolution);
    ledcSetup(config.channelIN2, config.freq, config.resolution);
    ledcSetup(config.channelIN3, config.freq, config.resolution);
    ledcSetup(config.channelIN4, config.freq, config.resolution);

    ledcAttachPin(config.pinIN1, config.channelIN1);
    ledcAttachPin(config.pinIN2, config.channelIN2);
    ledcAttachPin(config.pinIN3, config.channelIN3);
    ledcAttachPin(config.pinIN4, config.channelIN4);

    stop();
}

void DRV8833::setMotorSpeed(int right, int left)
{
    right = constrain(right, -int(MAX_SPEED), int(MAX_SPEED));
    left  = constrain(left, -int(MAX_SPEED), int(MAX_SPEED));

    rightSpeed = right;
    leftSpeed  = left;

    if (right > 0)
    {
        ledcWrite(config.channelIN1, abs(right));
        ledcWrite(config.channelIN2, 0);
    }
    else if (right < 0)
    {
        ledcWrite(config.channelIN1, 0);
        ledcWrite(config.channelIN2, abs(right));
    }
    else
    {
        ledcWrite(config.channelIN1, 0);
        ledcWrite(config.channelIN2, 0);
    }

    if (left > 0)
    {
        ledcWrite(config.channelIN3, abs(left));
        ledcWrite(config.channelIN4, 0);
    }
    else if (left < 0)
    {
        ledcWrite(config.channelIN3, 0);
        ledcWrite(config.channelIN4, abs(left));
    }
    else
    {
        ledcWrite(config.channelIN3, 0);
        ledcWrite(config.channelIN4, 0);
    }
}

void DRV8833::stop()
{
    setMotorSpeed(0, 0);
}