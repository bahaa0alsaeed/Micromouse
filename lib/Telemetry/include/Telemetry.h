#pragma once

#include "Values.h"
#include "Commands.h"

/**
 * Central configuration of the unified Telemetry library.
 *
 * Values are plain data: the library performs no dynamic allocation
 * and takes no ownership beyond storing the pointers/ports until
 * Telemetry::begin() is called.
 */
struct TelemetryConfig
{
    // Wi-Fi network to connect to.
    const char* ssid = nullptr;
    const char* password = nullptr;

    // Destination host for outgoing value packets, as a dotted-quad
    // string such as "192.168.1.100".
    const char* host = nullptr;

    // UDP port used for outgoing value packets.
    uint16_t telemetryPort = 0;

    // UDP port that receives incoming commands.
    uint16_t commandPort = 0;
};

/**
 * Unified entry point for the telemetry stack.
 *
 * The library is split into independent services:
 *
 * - values   streams registered variables over UDP (formerly Logger).
 * - commands receives command frames over UDP (formerly CommandReceiver).
 *
 * Telemetry is the sole owner of the Wi-Fi connection: begin() starts
 * the connection and update() keeps it alive and reconnects when it
 * drops, without ever blocking the application. The services never
 * touch Wi-Fi themselves; they are ticked by update() only while the
 * connection is up.
 *
 * Including this header exposes the complete public API, so normal
 * application code only needs:
 *
 *     #include <Telemetry.h>
 *
 * Intended usage:
 *
 *     Telemetry telemetry;
 *
 *     void setup() {
 *         telemetry.begin();
 *         telemetry.values.registerFast("speed", &speed);
 *         telemetry.commands.on("set_speed", speedHandler);
 *     }
 *
 *     void loop() {
 *         telemetry.update();
 *     }
 */
class Telemetry
{
public:
    Telemetry() = default;

    // Explicitly non-copyable: the services own UDP sockets and state.
    Telemetry(const Telemetry&) = delete;
    Telemetry& operator=(const Telemetry&) = delete;


    /**
     * Initializes the library from an explicit configuration,
     * which takes priority over the environment macros.
     */
    void begin(const TelemetryConfig& config);

    /**
     * Keeps the Wi-Fi connection alive and ticks every service.
     * Call repeatedly from loop().
     */
    void update();

    Values values;
    Commands commands;

private:
    bool initialized = false;

    // Wi-Fi state. Telemetry is the sole owner of the connection; the
    // services never touch Wi-Fi themselves.
    const char* wifiSsid = nullptr;
    const char* wifiPassword = nullptr;

    // Time in ms between reconnect attempts. A single attempt is cheap and
    // non-blocking, but the interval avoids re-issuing WiFi.begin() in every
    // loop iteration while the AP is unreachable.
    uint32_t reconnectInterval = 10000;

    uint32_t lastWifiAttempt = 0;
};

