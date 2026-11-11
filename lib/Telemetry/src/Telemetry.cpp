#include "Telemetry.h"

#include <Arduino.h>
#include "WiFi.h"


void Telemetry::begin(const TelemetryConfig& config)
{
    // Convert the configured destination address from text to an IP
    // address. Networking is intentionally not started on failure so a
    // typo in the configuration cannot cause a half-initialized library.
    IPAddress destination;

    if (!destination.fromString(config.host))
    {
        Serial.println("Telemetry: invalid host address");
        return;
    }

    // Start the connection. Completion is observed non-blocking in
    // update(); nothing here waits for the link.
    wifiSsid = config.ssid;
    wifiPassword = config.password;

    WiFi.begin(wifiSsid, wifiPassword);
    lastWifiAttempt = millis();

    values.begin(destination, config.telemetryPort);
    commands.begin(config.commandPort);

    initialized = true;
}

void Telemetry::update()
{
    if (!initialized)
        return;

    // While offline, keep trying to reconnect at most once per interval
    // and tick nothing. Never blocks the application.
    if (WiFi.status() != WL_CONNECTED)
    {
        if (millis() - lastWifiAttempt >= reconnectInterval)
        {
            WiFi.begin(wifiSsid, wifiPassword);
            lastWifiAttempt = millis();
        }

        return;
    }

    // Connected: tick every service.
    values.update();
    commands.update();
}
