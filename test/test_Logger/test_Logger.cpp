#include <Arduino.h>
#include "Logger.h"
#include <cmath>

// ---------------------------------------------------------
// Test variables
// ---------------------------------------------------------

float sineWave = 0.0f;
float ramp = 0.0f;
float constant = 50.0f;
float squareWave = 0.0f;

// ---------------------------------------------------------
// Setup
// ---------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    delay(1000);

    logger.begin();

    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.localIP());

    logger.setUpdateInterval(50);

    logger.registerVariable("sine", &sineWave);
    logger.registerVariable("ramp", &ramp);
    logger.registerVariable("constant", &constant);
    logger.registerVariable("square", &squareWave);

}
// ---------------------------------------------------------
// Loop
// ---------------------------------------------------------

void loop()
{
    static uint32_t startTime = micros();

    const float t = (micros() - startTime) / 1000000.0f;

    // Sine wave: ±100
    sineWave = 100.0f * sinf(2.0f * PI * 0.5f * t);

    // Ramp: 0 → 100 → 0
    ramp = fmodf(t * 20.0f, 200.0f);

    if (ramp > 100.0f)
        ramp = 200.0f - ramp;

    // Constant value
    constant = 50.0f;

    // Square wave: ±100
    squareWave = (fmodf(t, 2.0f) < 1.0f) ? 100.0f : -100.0f;

    logger.update();

    delay(1);
}