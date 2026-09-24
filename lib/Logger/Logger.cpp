#include "Logger.h"

Logger logger;

const char* WIFI_SSID = MY_WIFI_SSID;
const char* WIFI_PASSWORD = MY_WIFI_PASSWORD;
const char* LOGGER_HOST = MY_LOGGER_HOST;
constexpr uint16_t LOGGER_PORT = MY_LOGGER_PORT;

Logger::Logger() = default;

void Logger::begin()
{
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    {
        delay(100);
    }

    IPAddress destination;

    // Convert the configured destination address from text to an IP address.
    if (!destination.fromString(LOGGER_HOST))
    {
        Serial.println("Invalid LOGGER_HOST");
        return;
    }

    setDestination(destination, LOGGER_PORT);
}

void Logger::setLevel(LogLevel level)
{
    Logger::level = level;
}

void Logger::setDestination(const IPAddress& ip, uint16_t port)
{
    destinationIP = ip;
    destinationPort = port;
}

void Logger::sendPacket(
    const std::vector<LogVariable>& variables)
{
    if (variables.empty())
        return;

    std::string message;

    for (size_t i = 0; i < variables.size(); i++)
    {
        message += variables[i].name;
        message += '=';
        message += std::to_string(*variables[i].variable);

        if (i + 1 < variables.size())
            message += ',';
    }

    message += '\n';

    udp.beginPacket(destinationIP, destinationPort);

    udp.write(reinterpret_cast<const uint8_t*>(message.data()), message.size());

    udp.endPacket();
}

void Logger::setFastInterval(const uint32_t ms)
{
    if (ms > 0)
        fastInterval = ms;
}

void Logger::setSlowInterval(const uint32_t ms)
{
    if (ms > 0)
        slowInterval = ms;
}

void Logger::registerFastVariable(
    const std::string& name,
    const float* variable)
{
    fastVariables.push_back({name, variable});
}

void Logger::registerSlowVariable(
    const std::string& name,
    const float* variable)
{
    slowVariables.push_back({name, variable});
}

void Logger::update()
{
    if (WiFi.status() != WL_CONNECTED)
        return;

    if (destinationPort == 0)
        return;

    const uint32_t now = millis();

    // Send the packet when it's time comes.
    if (!fastVariables.empty() && now - lastFastUpdate >= fastInterval)
    {
        lastFastUpdate = now;
        sendPacket(fastVariables);
    }

    if (!slowVariables.empty() && now - lastSlowUpdate >= slowInterval)
    {
        lastSlowUpdate = now;
        sendPacket(slowVariables);
    }
}