#pragma once


class IWallSensors
{
public:
    virtual ~IWallSensors() = default;

    virtual bool wallFront() = 0;
    virtual bool wallRight() = 0;
    virtual bool wallLeft() = 0;
};