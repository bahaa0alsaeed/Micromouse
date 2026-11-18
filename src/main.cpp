// #include <Arduino.h>
// #include <WiFi.h>
// #include <Telemetry.h>
//
// Telemetry telemetry;
//
// TelemetryConfig config {
//     .ssid = MY_WIFI_SSID,
//     .password = MY_WIFI_PASSWORD,
//     .host = MY_TELEMETRY_HOST,
//     .telemetryPort = MY_TELEMETRY_PORT,
//     .commandPort = MY_COMMANDS_PORT,
//     .logPort = MY_LOGS_PORT
// };
//
// // Telemetry signals
// float sine = 0.0f;
// float slowWave = 0.0f;
// float square = 0.0f;
//
// // Signal generation
// constexpr float TWO_PI_F = 6.28318530718f;
//
// constexpr uint32_t SINE_PERIOD_MS = 4000;
// constexpr uint32_t SLOW_PERIOD_MS = 30000;
// constexpr uint32_t SQUARE_PERIOD_MS = 4000;
//
// bool signalsRunning = true;
// uint32_t signalStartMs = 0;
//
// // Commands
// void stopSignals(const Command& cmd)
// {
//     Serial.printf(
//         "[EXECUTION] stop at %lu ms\n",
//         millis()
//     );
//     signalsRunning = false;
// }
//
// void startSignals(const Command& cmd)
// {
//     signalsRunning = true;
//     signalStartMs = millis();
//
//     Serial.println("[Command] Signals started");
// }
//
// void setup()
// {
//     Serial.begin(115200);
//
//     Serial.println("\nStarting Telemetry test...");
//
//     telemetry.begin(config);
//
//     // Register telemetry values
//     telemetry.stream.addFast("sine", &sine);
//     telemetry.stream.addFast("slow_wave", &slowWave);
//     telemetry.stream.addFast("square", &square);
//
//     // Register commands
//     telemetry.commands.on("stop", stopSignals);
//     telemetry.commands.on("start", startSignals);
//
//     telemetry.logs.info("HI");
//
//     signalStartMs = millis();
//
//     Serial.println("Telemetry initialization completed");
// }
//
// void updateSignals()
// {
//     if (!signalsRunning)
//         return;
//
//     const uint32_t elapsed = millis() - signalStartMs;
//
//     // 1. Sine wave: -1 to +1
//     const float sinePhase =
//         TWO_PI_F * (elapsed % SINE_PERIOD_MS) / SINE_PERIOD_MS;
//
//     sine = 90 * sinf(sinePhase);
//
//     // 2. Slow triangle wave: 0 to 100
//     const float slowPhase =
//         static_cast<float>(elapsed % SLOW_PERIOD_MS) / SLOW_PERIOD_MS;
//
//     slowWave = (slowPhase < 0.5f)
//         ? slowPhase * 200.0f
//         : (1.0f - slowPhase) * 200.0f;
//
//     // 3. Square wave: 0 or 100
//     square = ((elapsed / (SQUARE_PERIOD_MS / 2)) % 2)
//         ? 100.0f
//         : 0.0f;
// }
//
// void loop()
// {
//     telemetry.update();
//
//     updateSignals();
//
//     static uint32_t lastStatus = 0;
//     static uint32_t lastLog = 0;
//
//     if (millis() - lastLog > 500)
//     {
//         lastLog = millis();
//         telemetry.logs.info("HI");
//     }
//
//     if (millis() - lastStatus >= 2000)
//     {
//         lastStatus = millis();
//
//         Serial.printf("WiFi status: %d\n", WiFi.status());
//         Serial.printf("ESP32 IP: %s\n", WiFi.localIP().toString().c_str());
//     }
//
//
// }
//
//











#include <Arduino.h>
#include <WiFi.h>
#include <Telemetry.h>
#include "Logger.h"
Telemetry telemetry;

// ============================================================
// Configuration
// ============================================================

TelemetryConfig config {
    .ssid = MY_WIFI_SSID,
    .password = MY_WIFI_PASSWORD,
    .host = MY_TELEMETRY_HOST,
    .streamerPort = MY_STREAMER_PORT,
    .commandPort = MY_COMMANDS_PORT,
    .logPort = MY_LOGS_PORT
};

// ============================================================
// Telemetry Values
// ============================================================

// Fast values
float sine = 0.0f;
float square = 0.0f;
float targetSpeed = 0.0f;

// Slow values
float triangle = 0.0f;
float battery = 4.20f;

// ============================================================
// Signal Configuration
// ============================================================

constexpr float TWO_PI_F = 6.28318530718f;

constexpr uint32_t SINE_PERIOD_MS = 4000;
constexpr uint32_t SQUARE_PERIOD_MS = 2000;
constexpr uint32_t TRIANGLE_PERIOD_MS = 30000;

float amplitude = 100.0f;

bool signalsRunning = true;

uint32_t signalStartMs = 0;

// ============================================================
// Test State
// ============================================================

uint32_t lastLog = 0;
uint32_t lastStatus = 0;
uint32_t lastBatteryUpdate = 0;

uint32_t commandCount = 0;

// ============================================================
// Command Handlers
// ============================================================

void startSignals(const Command& cmd)
{
    signalsRunning = true;
    signalStartMs = millis();

    telemetry.logs.info("Signals started");

    Serial.println("[COMMAND] Signals started");
}

void stopSignals(const Command& cmd)
{
    signalsRunning = false;

    telemetry.logs.info("Signals stopped");

    Serial.println("[COMMAND] Signals stopped");
}

void setSpeed(const Command& cmd)
{
    if (cmd.valueCount != 1)
    {
        telemetry.logs.info("set_speed requires exactly one value");
        return;
    }

    targetSpeed = cmd.values[0];

    Serial.printf(
        "[COMMAND] Target speed: %.2f\n",
        targetSpeed
    );

    telemetry.logs.info("Target speed updated");
}

void setAmplitude(const Command& cmd)
{
    if (cmd.valueCount != 1)
    {
        telemetry.logs.info("set_amplitude requires exactly one value");
        return;
    }

    amplitude = constrain(cmd.values[0], 0.0f, 100.0f);

    Serial.printf(
        "[COMMAND] Amplitude: %.2f\n",
        amplitude
    );

    telemetry.logs.info("Amplitude updated");
}

// ============================================================
// Signal Generation
// ============================================================

void updateSignals()
{
    if (!signalsRunning)
        return;

    const uint32_t elapsed = millis() - signalStartMs;

    // Sine wave: -amplitude to +amplitude
    const float sinePhase =
        TWO_PI_F *
        static_cast<float>(elapsed % SINE_PERIOD_MS) /
        SINE_PERIOD_MS;

    sine = amplitude * sinf(sinePhase);

    // Square wave: 0 or amplitude
    square = ((elapsed / (SQUARE_PERIOD_MS / 2)) % 2)
        ? amplitude
        : 0.0f;

    // Triangle wave: 0 to 100
    const float trianglePhase =
        static_cast<float>(elapsed % TRIANGLE_PERIOD_MS) /
        TRIANGLE_PERIOD_MS;

    triangle = (trianglePhase < 0.5f)
        ? trianglePhase * 200.0f
        : (1.0f - trianglePhase) * 200.0f;
}

// ============================================================
// Battery Simulation
// ============================================================

void updateBattery()
{
    const uint32_t now = millis();

    if (now - lastBatteryUpdate < 1000)
        return;

    lastBatteryUpdate = now;

    // Simulate gradual battery discharge.
    battery -= 0.001f;

    if (battery < 3.30f)
        battery = 4.20f;
}

// ============================================================
// Setup
// ============================================================

void setup()
{

    Serial.begin(115200);

    delay(500);

    Serial.println();
    Serial.println("================================");
    Serial.println("       TELEMETRY TEST");
    Serial.println("================================");

    // Initialize Telemetry and its services.
    telemetry.begin(config);

    Serial.println("[INIT] Telemetry initialized");

    // --------------------------------------------------------
    // Configure update intervals
    // --------------------------------------------------------

    telemetry.stream.setFastInterval(50);
    telemetry.stream.setSlowInterval(500);

    // --------------------------------------------------------
    // Register Fast Values
    // --------------------------------------------------------

    telemetry.stream.addFast("sine", &sine);
    telemetry.stream.addFast("square", &square);
    telemetry.stream.addFast("target_speed", &targetSpeed);

    // --------------------------------------------------------
    // Register Slow Values
    // --------------------------------------------------------

    telemetry.stream.addSlow("triangle", &triangle);
    telemetry.stream.addSlow("battery", &battery);

    Serial.println("[INIT] Values registered");

    // --------------------------------------------------------
    // Register Commands
    // --------------------------------------------------------

    telemetry.commands.on("start", startSignals);
    telemetry.commands.on("stop", stopSignals);
    telemetry.commands.on("set_speed", setSpeed);
    telemetry.commands.on("set_amplitude", setAmplitude);

    Serial.println("[INIT] Commands registered");

    // --------------------------------------------------------
    // Initial Logs
    // --------------------------------------------------------

    telemetry.logs.info("Telemetry test started");
    telemetry.logs.info("Fast and Slow values registered");
    telemetry.logs.info("Command handlers registered");

    signalStartMs = millis();

    Serial.println("[INIT] Setup completed");
}

// ============================================================
// Diagnostics
// ============================================================

void updateDiagnostics()
{
    const uint32_t now = millis();

    if (now - lastStatus < 5000)
        return;

    lastStatus = now;

    Serial.println();
    Serial.println("---------- STATUS ----------");

    Serial.printf(
        "WiFi status: %d\n",
        WiFi.status()
    );

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.printf(
            "ESP32 IP: %s\n",
            WiFi.localIP().toString().c_str()
        );
    }

    Serial.printf(
        "Signals: %s\n",
        signalsRunning ? "RUNNING" : "STOPPED"
    );

    Serial.printf(
        "Target speed: %.2f\n",
        targetSpeed
    );

    Serial.printf(
        "Amplitude: %.2f\n",
        amplitude
    );

    Serial.println("----------------------------");
}

// ============================================================
// Periodic Logging
// ============================================================

void updateLogs()
{
    const uint32_t now = millis();

    if (now - lastLog < 3000)
        return;

    lastLog = now;

    telemetry.logs.info("Telemetry is running");
}

// ============================================================
// Main Loop
// ============================================================

void loop()
{
    // Always update Telemetry.
    telemetry.update();

    // Update test signals.
    updateSignals();

    // Update simulated battery.
    updateBattery();

    // Periodic diagnostics.
    updateDiagnostics();

    // Periodic logging.
    updateLogs();
}
