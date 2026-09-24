#pragma once

#include <string>
#include <vector>
#include "WiFi.h"
#include "WiFiUdp.h"

extern const char* WIFI_SSID;
extern const char* WIFI_PASSWORD;

enum class LogLevel
{
    OFF,
    NORMAL,
    DEBUG
};

/**
 * Sends registered floating-point variables over UDP.
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
 * The logger therefore reads the current value each time a packet is sent
 * rather than storing a copy of the value.
 *
 * Outgoing packets contain comma-separated name=value pairs.
 */
class Logger
{
    /**
     * The variable is referenced through a pointer so that the logger
     * always reads its current value when constructing a packet.
     */
    struct LogVariable
    {
        const std::string name;
        const float* variable;
    };

    LogLevel level = LogLevel::NORMAL;

    // Independent update intervals for the two variable groups.
    uint32_t fastInterval = 50;
    uint32_t slowInterval = 500;

    uint32_t lastFastUpdate = 0;
    uint32_t lastSlowUpdate = 0;

    std::vector<LogVariable> fastVariables;
    std::vector<LogVariable> slowVariables;

    WiFiUDP udp;
    IPAddress destinationIP;
    uint16_t destinationPort = 0;

    void setDestination(const IPAddress& ip, uint16_t port);

    /**
    * Values are formatted as:
    *
    *   name=value,name=value,...
    *
    * followed by a newline character.
    */
    void sendPacket(const std::vector<LogVariable>& variables);

public:
    Logger();

    // Use it inside setup().
    void begin();

    void setFastInterval(uint32_t ms);
    void setSlowInterval(uint32_t ms);

    void setLevel(LogLevel level);

    // Registers a variable in fast or slow update group.
    void registerFastVariable(const std::string& name, const float* variable);
    void registerSlowVariable(const std::string& name, const float* variable);

    /**
     * It checks whether it is time to send the next
     * fast-update or slow-update packet and sends it if so.
     */
    void update();
};

extern Logger logger;