#include "WallSensors.h"

WallSensors::WallSensors(const WallSensorConfig& config) :
    frontSensor(config.SDA,config.SCL,config.frontXshutPin),
    leftSensor(config.SDA,config.SCL,config.leftXshutPin),
    rightSensor(config.SDA,config.SCL,config.rightXshutPin)
{
}

WallReadings WallSensors::getReadings()
{
    return {};
}

bool WallSensors::wallFront()
{
    return false;
}

bool WallSensors::wallRight()
{
    return false;
}

bool WallSensors::wallLeft()
{
    return false;
}