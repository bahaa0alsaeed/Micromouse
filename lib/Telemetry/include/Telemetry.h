/**
 * @file Telemetry.h
 *
 * @brief Unified entry point for the telemetry stack (streaming,
 * commands and logging over UDP).
 *
 * The library is split into the independent services Streamer,
 * Commands and Logger, all owned by the Telemetry class. Including
 * this header exposes the complete public API, so normal application
 * code only needs:
 *
 *     #include <Telemetry.h>
 */

#pragma once

#include "Streamer.h"
#include "Commands.h"
#include "Logger.h"

/**
 * @brief Configuration parameters for the Telemetry library.
 *
 * This structure contains the network credentials and UDP endpoints
 * used to initialize the telemetry services.
 *
 * The library stores the provided string pointers without copying
 * their contents. The pointed-to strings must remain valid for as
 * long as the Telemetry instance may use them.
 */
struct TelemetryConfig
{
    // Wi-Fi network credentials.
    const char* ssid = nullptr;
    const char* password = nullptr;

    // Destination IPv4 address for outgoing UDP packets.
    const char* host = nullptr;

    // UDP destination port for streamed values.
    uint16_t streamerPort = 0;

    // UDP local port for receiving commands.
    uint16_t commandPort = 0;

    // UDP destination port for outgoing log messages.
    uint16_t logPort = 0;
};

/**
 * @brief Unified entry point for the telemetry stack.
 *
 * The library is split into independent services:
 *
 * - stream: Streams registered variables over UDP.
 * - commands: Receives and dispatches incoming command frames.
 * - logs: Buffers and transmits textual log messages over UDP.
 *
 * Telemetry is the sole owner of the Wi-Fi connection: begin() starts
 * the connection and update() keeps it alive and reconnects when it
 * drops, without ever blocking the application. The services never
 * touch Wi-Fi themselves; they are ticked by update() only while the
 * connection is up.
 *
 * Intended usage:
 *
 * Telemetry telemetry;
 *
 * void setup()
 * {
 *     static const TelemetryConfig config{
 *         .ssid = "MyNetwork",
 *         .password = "MyPassword",
 *         .host = "192.168.1.100",
 *         .streamerPort = 9000,
 *         .commandPort = 9001,
 *         .logPort = 9002
 *     };
 *
 *     telemetry.begin(config);
 *
 *     telemetry.stream.addFast("speed", &speed);
 *     telemetry.commands.on("set_speed", speedHandler);
 *     telemetry.logs.info("Robot initialized");
 * }
 *
 * void loop()
 * {
 *     telemetry.update();
 * }
 */
class Telemetry
{
public:
    Telemetry() = default;

    // Explicitly non-copyable: the services own UDP sockets and state.
    Telemetry(const Telemetry&) = delete;
    Telemetry& operator=(const Telemetry&) = delete;


    /**
     * @brief Initializes the telemetry services using the provided
     *        configuration.
     *
     * Validates the destination IP address and UDP ports, configures all
     * telemetry services, and starts a non-blocking Wi-Fi connection attempt.
     *
     * Initialization stops if the destination address or any service
     * configuration is invalid.
     *
     * A successful return indicates that all services were successfully
     * initialized and the Wi-Fi connection attempt was started, not that
     * the Wi-Fi connection has been established.
     *
     * @param config Network credentials and UDP endpoint configuration.
     * @return true if all services were successfully initialized,
     *         false if the destination IP address or any service
     *         configuration is invalid.
     */
    bool begin(const TelemetryConfig& config);

    /**
     * @brief Maintains the Wi-Fi connection and updates all services.
     *
     * While disconnected, periodically retries the Wi-Fi connection
     * and skips all service updates. Call repeatedly from loop().
     */
    void update();

    // Streams registered variables over UDP.
    Streamer stream;
    // Receives command frames over UDP.
    Commands commands;
    // Buffers textual log messages and sends them over UDP.
    Logger logs;

private:
    // True after successful initialization.
    bool m_initialized = false;

    // Credentials used for Wi-Fi connection and reconnection.
    const char* m_wifiSsid = nullptr;
    const char* m_wifiPassword = nullptr;

    // Minimum interval between Wi-Fi connection attempts, in milliseconds.
    uint32_t m_reconnectInterval = 10000;

    // Timestamp of the last Wi-Fi connection attempt, in milliseconds.
    uint32_t m_lastWifiAttempt = 0;
};
