/**
 * @file Streamer.h
 *
 * @brief Streams registered floating-point variables over UDP.
 *
 * Part of the Telemetry library. Applications access this service
 * through Telemetry::stream. The service does not manage the Wi-Fi
 * connection; connection management is handled by Telemetry.
 */

#pragma once

#include <string>
#include <vector>

#include "IPAddress.h"
#include "WiFiUdp.h"


/**
 * @brief Streams registered floating-point variables over UDP.
 *
 * Variables are divided into two independently scheduled groups:
 *
 * - Fast: Intended for frequently updated values, such as control-loop
 *   measurements and real-time graph data.
 *
 * - Slow: Intended for less frequently changing values, such as
 *   configuration parameters, PID gains, and status information.
 *
 * Each group has its own update interval. Registered variables are
 * stored as pointers, and their current values are read when a packet
 * is constructed. The pointed-to variables must remain valid while
 * they are registered.
 *
 * Each outgoing packet contains comma-separated name=value pairs,
 * terminated by a newline character.
 *
 * The service does not manage the Wi-Fi connection; Telemetry handles
 * connection management and periodically updates this service.
 */
class Streamer
{
public:
    /**
     * @brief Sets the update interval of the fast variable group.
     *
     * @param ms Interval between fast-update packets in milliseconds.
     *           Zero is ignored and keeps the current interval.
     *           Default: 120 ms.
     */
    void setFastInterval(uint32_t ms);

    /**
     * @brief Sets the update interval of the slow variable group.
     *
     * @param ms Interval between slow-update packets in milliseconds.
     *           Zero is ignored and keeps the current interval.
     *           Default: 500 ms.
     */
    void setSlowInterval(uint32_t ms);


    /**
     * @brief Registers a variable in the fast update group.
     *
     * The variable is referenced by pointer; its current value is read
     * whenever a packet is constructed. The pointed-to value must remain
     * valid while registered.
     *
     * Duplicate names are allowed.
     *
     * @param name Name used to identify the streamed value. Must not be
     *             empty or contain ',', '=', '\n', or '\r'.
     * @param variable Pointer to the value to stream. Must not be null.
     * @return true if the variable was registered; false if the name is
     *         invalid or the pointer is null.
     */
    bool addFast(const std::string& name, const float* variable);

    /**
     * @brief Registers a variable in the  update group.
     *
     * The variable is referenced by pointer; its current value is read
     * whenever a packet is constructed. The pointed-to value must remain
     * valid while registered.
     *
     * Duplicate names are allowed.
     *
     * @param name Name used to identify the streamed value. Must not be
     *             empty or contain ',', '=', '\n', or '\r'.
     * @param variable Pointer to the value to stream. Must not be null.
     * @return true if the variable was registered; false if the name is
     *         invalid or the pointer is null.
     */
    bool addSlow(const std::string& name, const float* variable);


    // Explicitly non-copyable: the service owns a UDP socket and runtime state.
    Streamer(const Streamer&) = delete;
    Streamer& operator=(const Streamer&) = delete;

private:
    friend class Telemetry;

    Streamer() = default;

    /**
     * @brief One registered variable in an update group.
     *
     * The variable is referenced through a pointer so that the streamer
     * always reads its current value when constructing a packet.
     */
    struct Variable
    {
        // Streamed variable name.
        std::string name;

        // Non-owning pointer to the source value.
        const float* variable;
    };


    // Fast-group update interval in milliseconds.
    uint32_t m_fastInterval = 120;
    // Slow-group update interval in milliseconds.
    uint32_t m_slowInterval = 500;

    // Timestamp of the last successfully transmitted fast-group packet.
    uint32_t m_lastFastTransmit = 0;
    // Timestamp of the last successfully transmitted slow-group packet.
    uint32_t m_lastSlowTransmit = 0;

    // Registered variables of the fast update group.
    std::vector<Variable> m_fastVariables;
    // Registered variables of the slow update group.
    std::vector<Variable> m_slowVariables;

    // UDP socket used for the outgoing value packets.
    WiFiUDP m_udp;
    // Destination address for outgoing value packets.
    IPAddress m_destinationIP;
    // Destination port; zero means not configured and nothing is sent.
    uint16_t m_destinationPort = 0;

    /**
     * @brief Configures the destination for outgoing UDP data packets.
     *
     * @param destination Destination IP address. Must not be 0.0.0.0.
     * @param port Destination UDP port. Must not be zero.
     *
     * @return true if the destination was successfully configured,
     *         false if the IP address or port is invalid.
     */
    bool begin(const IPAddress& destination, uint16_t port);

    /**
     * @brief Builds and transmits one UDP packet from a variable group.
     *
     * Values are read from their registered pointers when the packet
     * is constructed. Variables are included in registration order.
     *
     * Each entry is formatted as name=value, entries are separated
     * by commas, and the packet ends with a newline character.
     *
     * @param variables Registered variables to include in the packet.
     * @return true if the complete packet was successfully submitted
     *         to the UDP stack; false if packet transmission failed.
     */
    bool sendPacket(const std::vector<Variable>& variables);

    /**
     * @brief Checks the transmission schedule of both variable groups.
     *
     * Each group is scheduled independently and may transmit one packet
     * per update() call when its interval has elapsed.
     *
     * No packets are sent when the destination is not configured or
     * when the registered variables vector is empty.
     *
     * Call periodically through Telemetry::update().
     */
    void update();
};
