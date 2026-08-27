#pragma once

struct EncoderConfig
{
    unsigned int pin;
    unsigned int ticksPerRevolution;
    float wheelDiameter;
    float gearRatio;
};

class Encoder
{
    void handleInterrupt();
    void update();

    EncoderConfig config{};

    int count = 0;
    int previousCount = 0;
    int deltaCount = 0;
    unsigned int previousTime = 0;
    unsigned int deltaTime = 0;
    float distance = 0.0;
    float velocity = 0.0;
    bool error = false;

public:
    explicit Encoder(const EncoderConfig& config);
    float getDistance();
    float getVelocity();
    void reset();
    bool hasError();
};
