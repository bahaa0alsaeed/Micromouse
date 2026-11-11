#include "Values.h"

#include <Arduino.h>


void Values::setFastInterval(const uint32_t ms)
{
    if (ms > 0)
        fastInterval = ms;
}

void Values::setSlowInterval(const uint32_t ms)
{
    if (ms > 0)
        slowInterval = ms;
}

void Values::registerFast(const std::string& name, const float* variable)
{
    fastVariables.push_back({name, variable});
}

void Values::registerSlow(const std::string& name, const float* variable)
{
    slowVariables.push_back({name, variable});
}

// Compatibility aliases for the original Logger API.

void Values::begin(const IPAddress& destination, const uint16_t port)
{
    destinationIP = destination;
    destinationPort = port;
}

void Values::sendPacket(const std::vector<Variable>& variables)
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

    bool beginResult = udp.beginPacket(destinationIP, destinationPort);

    size_t written = udp.write(reinterpret_cast<const uint8_t*>(message.data()), message.size());

    bool endResult = udp.endPacket();

}

void Values::update()
{
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
