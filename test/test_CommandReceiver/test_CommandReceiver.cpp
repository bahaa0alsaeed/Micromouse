#include <Arduino.h>
#include <Telemetry.h>

Telemetry telemetry;

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
    if (command.valueCount != 1)
    {
        Serial.println("Blink_Speed received without a value");
        return;
    }

    blinkInterval = command.values[0];
    Serial.print("Blink_Speed received: ");
    Serial.println(command.values[0]);

}

// ---------------------------------------------------------
// Setup
// ---------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    delay(1000);

    // Without arguments the default configuration is used, whose command
    // port is MY_LOGGER_PORT — the same port as before.
    telemetry.begin();

    telemetry.commands.on("LED_ON", handleLedOn);
    telemetry.commands.on("LED_OFF", handleLedOff);
    telemetry.commands.on("Blink_Speed", handleBlinkSpeed);

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
    telemetry.update();

    if (isBlinking && millis() - t >= blinkInterval)
    {
        t = millis();
        digitalWrite(2, !digitalRead(2));
    }

    delay(1);
}
