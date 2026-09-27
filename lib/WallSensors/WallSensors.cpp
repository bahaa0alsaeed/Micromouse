#include "WallSensors.h"


WallSensors::WallSensors(const WallSensorConfig& config)
    : config(config)
{
}


TwoWire* WallSensors::getBus(const WallSensorBusConfig& busConfig) const
{
    for (uint8_t i = 0; i < busCount; ++i)
    {
        if (buses[i].SDA == busConfig.SDA && buses[i].SCL == busConfig.SCL)
        {
            return buses[i].wire;
        }
    }

    return nullptr;
}


TwoWire* WallSensors::createBus(const WallSensorBusConfig& busConfig)
{
    if (busCount >= MAX_BUSES)
        return nullptr;

    TwoWire* wire = nullptr;

    if (busCount == 0)
        wire = &Wire;
    else
        wire = &Wire1;

    wire->begin(
        busConfig.SDA,
        busConfig.SCL);

    buses[busCount].wire = wire;
    buses[busCount].SDA = busConfig.SDA;
    buses[busCount].SCL = busConfig.SCL;

    ++busCount;

    return wire;
}


bool WallSensors::initializeSensor(
    VL53L0X& sensor,
    TwoWire& wire,
    const uint8_t xshutPin,
    const uint8_t address)
{
    digitalWrite(xshutPin, HIGH);

    sensor.setBus(&wire);

    if (!sensor.init())
        return false;

    sensor.setAddress(address);

    sensor.setMeasurementTimingBudget(20000);
    sensor.startContinuous(0);

    return true;
}


bool WallSensors::begin()
{
    pinMode(config.frontXshutPin, OUTPUT);
    pinMode(config.leftXshutPin, OUTPUT);
    pinMode(config.rightXshutPin, OUTPUT);

    digitalWrite(config.frontXshutPin, LOW);
    digitalWrite(config.leftXshutPin, LOW);
    digitalWrite(config.rightXshutPin, LOW);

    TwoWire* frontBus = getBus(config.frontBus);

    if (frontBus == nullptr)
        frontBus = createBus(config.frontBus);

    TwoWire* leftBus = getBus(config.leftBus);

    if (leftBus == nullptr)
        leftBus = createBus(config.leftBus);

    TwoWire* rightBus = getBus(config.rightBus);

    if (rightBus == nullptr)
        rightBus = createBus(config.rightBus);

    if (frontBus == nullptr || leftBus == nullptr || rightBus == nullptr)
    {
        return false;
    }

    if (!initializeSensor(frontSensor, *frontBus, config.frontXshutPin, FRONT_ADDRESS))
    {
        return false;
    }

    if (!initializeSensor(leftSensor, *leftBus, config.leftXshutPin, LEFT_ADDRESS))
    {
        return false;
    }

    if (!initializeSensor(rightSensor, *rightBus, config.rightXshutPin, RIGHT_ADDRESS))
    {
        return false;
    }

    initialized = true;

    return true;
}


bool WallSensors::isReady(VL53L0X& sensor)
{
    return (sensor.readReg(0x13) & 0x07) != 0;
}


bool WallSensors::readSensor(VL53L0X& sensor, float& reading)
{
    if (!isReady(sensor))
        return false;

    reading = static_cast<float>(sensor.readRangeContinuousMillimeters());

    return !sensor.timeoutOccurred();
}


void WallSensors::update()
{
    if (!initialized)
        return;

    float reading;

    if (readSensor(frontSensor, reading))
        readings.front = reading;

    if (readSensor(leftSensor, reading))
        readings.left = reading;

    if (readSensor(rightSensor, reading))
        readings.right = reading;
}


WallReadings WallSensors::getReadings() const
{
    return readings;
}


bool WallSensors::wallFront()
{
    return readings.front <= config.frontThreshold;
}


bool WallSensors::wallRight()
{
    return readings.right <= config.rightThreshold;
}


bool WallSensors::wallLeft()
{
    return readings.left <= config.leftThreshold;
}
