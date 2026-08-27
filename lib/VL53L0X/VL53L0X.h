#pragma once

class VL53L0X
{
    unsigned int distance = 0;
    bool ready = false;
    bool error = false;

public:
    VL53L0X(unsigned int SDA, // Edit it on UML
            unsigned int SCL,
            unsigned int shutPin);

    unsigned int readDistance();
    bool isReady();
    bool hasError();
};

