#include "Navigation.h"

Navigation::Navigation(
    DRV8833& driver,
    WallSensors& sensors,
    Encoder& leftEncoder,
    Encoder& rightEncoder,
    PID& positionPID,
    PID& leftSpeedPID,
    PID& rightSpeedPID,
    const NavigationConfig& config
)
    : driver(driver),
      leftEncoder(leftEncoder),
      rightEncoder(rightEncoder),
      sensors(sensors),
      positionPID(positionPID),
      leftSpeedPID(leftSpeedPID),
      rightSpeedPID(rightSpeedPID),
      config(config)
{
}

void Navigation::update(float deltaTime)
{
}

void Navigation::moveForward(float speed, float distance)
{
}

void Navigation::turnLeft()
{
}

void Navigation::turnLeft45()
{
}

void Navigation::turnRight()
{
}

void Navigation::turnRight45()
{
}

void Navigation::turnBack()
{
}

void Navigation::stop()
{
}

bool Navigation::isMoving() const
{
    return false;
}

bool Navigation::isAtTarget() const
{
    return false;
}

bool Navigation::hasError() const
{
    return false;
}

void Navigation::updateSensors()
{
}

float Navigation::updatePositionControl(float deltaTime)
{
    return 0.0f;
}

void Navigation::updateSpeedControl(
    float leftTarget,
    float rightTarget,
    float deltaTime
)
{
}

void Navigation::applyMotorOutput()
{
}

void Navigation::resetControllers()
{
}