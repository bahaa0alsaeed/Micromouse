#include <Arduino.h>
#include <Telemetry.h>
#include <cmath>

Telemetry telemetry;

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

    telemetry.begin();

    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.localIP());

    telemetry.stream.setFastInterval(50);
    telemetry.stream.setSlowInterval(500);

    telemetry.stream.addFast("sine", &sineWave);
    telemetry.stream.addFast("ramp", &ramp);

    telemetry.stream.addSlow("constant", &constant);
    telemetry.stream.addSlow("square", &squareWave);
}

// ---------------------------------------------------------
// Loop
// ---------------------------------------------------------

void loop()
{
    static const uint32_t startTime = micros();

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

    telemetry.update();

    delay(1);
}
