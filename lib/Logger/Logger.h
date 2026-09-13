#pragma once
#include "string"
#include "vector"
#include "WiFi.h"

extern const char* WIFI_SSID;
extern const char* WIFI_PASSWORD;

enum LogLevel
{
    OFF,
    NORMAL,
    DEBUG
};

class Logger
{
    struct logVariable
    {
        const std::string name;
        const float* variable;
    };

    LogLevel level = LogLevel::NORMAL;
    uint32_t interval = 500;
    uint32_t lastUpdate = 0;
    std::vector<logVariable> variables;

    WiFiUDP messageUdp;
    uint16_t messagePort = 0;

    WiFiUDP udp;
    IPAddress destinationIP;
    uint16_t destinationPort = 0;

    void setDestination(const IPAddress& ip, uint16_t port);

public:
    Logger();
    void begin();

    void setUpdateInterval(uint32_t ms);
    void setLevel(LogLevel level);

    void registerVariable(
        const std::string& name,
        const float* variable
        );
    void update();

};

extern Logger logger;

