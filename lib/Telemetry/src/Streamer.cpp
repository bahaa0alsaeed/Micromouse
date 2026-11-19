/**
 * @file Streamer.cpp
 *
 * @brief Implementation of the Streamer service.
 *
 * Builds comma-separated name=value packets and transmits them over
 * UDP on the two independent fast/slow update intervals.
 */

#include "Streamer.h"
#include <Arduino.h>


void Streamer::setFastInterval(const uint32_t ms)
{
    if (ms > 0)
        m_fastInterval = ms;
}

void Streamer::setSlowInterval(const uint32_t ms)
{
    if (ms > 0)
        m_slowInterval = ms;
}

bool Streamer::addFast(const std::string& name, const float* variable)
{
    if (variable == nullptr ||
        name.empty() ||
        name.find_first_of(",=\n\r") != std::string::npos)
    {
        return false;
    }

    m_fastVariables.push_back({name, variable});
    return true;
}

bool Streamer::addSlow(const std::string& name, const float* variable)
{
    if (variable == nullptr ||
        name.empty() ||
        name.find_first_of(",=\n\r") != std::string::npos)
    {
        return false;
    }

    m_slowVariables.push_back({name, variable});
    return true;
}


bool Streamer::begin(const IPAddress& destination, const uint16_t port)
{
    if (destination == IPAddress(0, 0, 0, 0) || port == 0)
        return false;

    m_destinationIP = destination;
    m_destinationPort = port;

    return true;
}

bool Streamer::sendPacket(const std::vector<Variable>& variables)
{
    if (variables.empty())
        return false;

    // Build "name=value,name=value,..." terminated by a newline.
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

    if (!m_udp.beginPacket(m_destinationIP, m_destinationPort))
        return false;

    const size_t written = m_udp.write(
        reinterpret_cast<const uint8_t*>(message.data()),
        message.size()
        );

    const int endResult = m_udp.endPacket();

    return written == message.size() && endResult == 1;

}

void Streamer::update()
{
    if (m_destinationPort == 0)
        return;

    const uint32_t now = millis();

    // Send the packet when it's time comes.
    if (!m_fastVariables.empty() && now - m_lastFastTransmit >= m_fastInterval)
    {
        if(sendPacket(m_fastVariables))
            m_lastFastTransmit = now;
    }

    if (!m_slowVariables.empty() && now - m_lastSlowTransmit >= m_slowInterval)
    {
        if (sendPacket(m_slowVariables))
            m_lastSlowTransmit = now;
    }
}
