#include <Arduino.h>
#include "CommandReceiver.h"

// ---------------------------------------------------------
// Command handlers
// ---------------------------------------------------------

uint32_t t = millis();
bool isBlinking = false;
uint32_t blinkInterval = 500;

void handleLedOn(const Command& command)
{
    Serial.println("LED_ON received");
    isBlinking = true;
}

void handleLedOff(const Command& command)
{
    Serial.println("LED_OFF received");
    digitalWrite(2, LOW);
    isBlinking = false;
}

void handleBlinkSpeed(const Command& command)
{
    if (!command.hasValue)
    {
        Serial.println("Blink_Speed received without value");
        return;
    }

    blinkInterval = command.value;
    Serial.print("Blink_Speed received: ");
    Serial.println(command.value);

}

// ---------------------------------------------------------
// Setup
// ---------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    delay(1000);

    commandReceiver.begin(MY_LOGGER_PORT);

    commandReceiver.onCommand("LED_ON", handleLedOn);
    commandReceiver.onCommand("LED_OFF", handleLedOff);
    commandReceiver.onCommand("Blink_Speed", handleBlinkSpeed);

    Serial.println("CommandReceiver test started");
    Serial.print("Listening on UDP port: ");
    Serial.println(MY_LOGGER_PORT);

    pinMode(2, OUTPUT);
}

// ---------------------------------------------------------
// Loop
// ---------------------------------------------------------

void loop()
{
    commandReceiver.update();

    if (isBlinking && millis() - t >= blinkInterval)
    {
        t = millis();
        digitalWrite(2, !digitalRead(2));
    }

    delay(1);
}