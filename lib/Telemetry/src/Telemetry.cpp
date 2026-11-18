/**
 * @file Telemetry.cpp
 *
 * @brief Implementation of the Telemetry facade.
 *
 * Owns the Wi-Fi connection: begin() starts it non-blocking and
 * update() keeps it alive and reconnects when it drops, without ever
 * blocking the application.
 */

#include "Telemetry.h"

#include <Arduino.h>
#include "WiFi.h"


bool Telemetry::begin(const TelemetryConfig& config)
{
    // Convert the configured destination address from text to an IP.
    IPAddress destination;

    if (!destination.fromString(config.host))
        return false;

    // Configure the UDP services.
    if (!stream.begin(destination, config.streamerPort))
        return false;

    if (!commands.begin(config.commandPort))
        return false;

    if (!logs.begin(destination, config.logPort))
        return false;

    // Start the Wi-Fi connection attempt without waiting for completion.
    m_wifiSsid = config.ssid;
    m_wifiPassword = config.password;

    WiFi.begin(m_wifiSsid, m_wifiPassword);
    m_lastWifiAttempt = millis();

    m_initialized = true;
    return true;
}

void Telemetry::update()
{
    if (!m_initialized)
        return;

    if (WiFi.status() != WL_CONNECTED)
    {
        // Retry only after the configured interval to avoid repeatedly
        // calling WiFi.begin() on every update cycle.
        if (millis() - m_lastWifiAttempt >= m_reconnectInterval)
        {
            WiFi.begin(m_wifiSsid, m_wifiPassword);
            m_lastWifiAttempt = millis();
        }

        return;
    }

    stream.update();
    commands.update();
    logs.update();
}
