#include "WallSensors.h"

WallSensors::WallSensors(const WallSensorConfig& config)
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