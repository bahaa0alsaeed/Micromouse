#pragma once


class INavigation
{
public:
    virtual ~INavigation() = default;

    virtual void moveForward(float distance, float speed = -1) = 0;
    virtual void turnLeft() = 0;
    virtual void turnRight() = 0;
};