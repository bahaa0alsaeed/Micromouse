#pragma once
#include "DRV8833.h"
#include "Encoder.h"
#include "WallSensors.h"
#include "PID.h"

struct NavigationConfig
{
    float wheelBase;
    float cellSize;
};

class Navigation
{
    DRV8833& driver;
    Encoder& leftEncoder;
    Encoder& rightEncoder;
    WallSensors& sensors;
    PID& positionPID;
    PID& leftSpeedPID;
    PID& rightSpeedPID;
    NavigationConfig config;

    void updateSensors();
    float updatePositionControl(float deltaTime);
    void updateSpeedControl(
        float leftTarget,
        float rightTarget,
        float deltaTime
    );
    void applyMotorOutput();
    void resetControllers();

public:
    Navigation(
        DRV8833& driver,
        WallSensors& sensors,
        Encoder& leftEncoder,
        Encoder& rightEncoder,
        PID& positionPID,
        PID& leftSpeedPID,
        PID& rightSpeedPID,
        const NavigationConfig& config
    );

    void update(float deltaTime);

    void moveForward(float speed, float distance = -1); // Edit it on UML

    void turnLeft();
    void turnLeft45();

    void turnRight();
    void turnRight45();

    void turnBack();

    void stop();

    [[nodiscard]] bool isMoving() const;
    [[nodiscard]] bool isAtTarget() const;
    [[nodiscard]] bool hasError() const;

};
