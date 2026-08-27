#pragma once

struct DRV8833Config
{
    unsigned int IN1;
    unsigned int IN2;
    unsigned int IN3;
    unsigned int IN4;
    unsigned int MAX_SPEED;
};

class DRV8833
{
    int rightSpeed = 0;
    int leftSpeed = 0;
    DRV8833Config config{};

public:
    explicit DRV8833(const DRV8833Config& config);
    void setMotorSpeed(int right, int left);
    void stop();
};

