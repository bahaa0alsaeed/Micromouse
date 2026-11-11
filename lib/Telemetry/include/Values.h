#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "WiFi.h"
#include "WiFiUdp.h"


/**
 * Streams registered floating-point variables over UDP.
 *
 * Variables are separated into two independent update groups:
 *
 * - Fast variables are intended for values that need frequent updates,
 *   such as control-loop data that is displayed as real-time graphs.
 *
 * - Slow variables are intended for values that change less frequently,
 *   such as configuration values, PID parameters, battery voltage,
 *   or other status information.
 *
 * Each registered variable is stored as a pointer to the original value.
 * The streamer therefore reads the current value each time a packet is
 * sent rather than storing a copy of the value.
 *
 * Outgoing packets contain comma-separated name=value pairs.
 *
 * Instantiation and socket setup are internal; applications interact with
 * this service through Telemetry::values. The service performs no Wi-Fi
 * management by itself.
 */
class Values
{
public:
    void setFastInterval(uint32_t ms);
    void setSlowInterval(uint32_t ms);


    // Registers a variable in the fast or slow update group.
    void registerFast(const std::string& name, const float* variable);
    void registerSlow(const std::string& name, const float* variable);

    /**
     * Checks whether it is time to send the next fast-update or
     * slow-update packet and sends it if so.
     */
    void update();

private:
    friend class Telemetry;

    /**
     * The variable is referenced through a pointer so that the streamer
     * always reads its current value when constructing a packet.
     */
    struct Variable
    {
        std::string name;
        const float* variable;
    };


    // Independent update intervals for the two variable groups.
    uint32_t fastInterval = 120;
    uint32_t slowInterval = 500;

    uint32_t lastFastUpdate = 0;
    uint32_t lastSlowUpdate = 0;

    std::vector<Variable> fastVariables;
    std::vector<Variable> slowVariables;

    WiFiUDP udp;
    IPAddress destinationIP;
    uint16_t destinationPort = 0;

    // Called once by Telemetry::begin() with the resolved destination.
    void begin(const IPAddress& destination, uint16_t port);

    /**
     * Values are formatted as:
     *
     *   name=value,name=value,...
     *
     * followed by a newline character.
     */
    void sendPacket(const std::vector<Variable>& variables);
};
