#include "Logger.h"

Logger logger;

const char* WIFI_SSID = MY_WIFI_SSID;
const char* WIFI_PASSWORD = MY_WIFI_PASSWORD;
const char* LOGGER_HOST = MY_LOGGER_HOST;
constexpr uint16_t LOGGER_PORT = MY_LOGGER_PORT;

Logger::Logger()
= default;

void Logger::begin()
{
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(100);
    }

    IPAddress destination;

    if (!destination.fromString(LOGGER_HOST))
    {
        Serial.println("Invalid LOGGER_HOST");
        return;
    }

    setDestination(destination, LOGGER_PORT);
}

void Logger::setUpdateInterval(const uint32_t ms)
{
    if (ms > 0)
        interval = ms;
}

void Logger::setLevel(LogLevel level)
{
    Logger::level = level;
}

void Logger::setDestination(const IPAddress& ip, uint16_t port)
{
    destinationIP = ip;
    destinationPort = port;
    messagePort = port + 1;
}

void Logger::registerVariable(
    const std::string& name,
    const float* variable)
{
    variables.push_back({name, variable});
}

void Logger::update()
{
    if (WiFi.status() != WL_CONNECTED)
        return;

    if (destinationPort == 0)
        return;

    if (variables.empty())
        return;

    const uint32_t now = millis();

    if (now - lastUpdate < interval)
        return;

    lastUpdate = now;

    std::string message;

    for (size_t i = 0; i < variables.size(); i++)
    {
        message += std::to_string(*variables[i].variable);

        if (i + 1 < variables.size())
            message += ',';
    }

    message += '\n';

    udp.beginPacket(destinationIP, destinationPort);
    udp.write(
        reinterpret_cast<const uint8_t*>(message.data()),
        message.size());
    udp.endPacket();
}

