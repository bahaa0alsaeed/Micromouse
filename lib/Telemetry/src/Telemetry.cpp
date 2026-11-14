#include "Telemetry.h"
#include <Arduino.h>
#include "WiFi.h"


bool Telemetry::begin(const TelemetryConfig& config)
{
    // Convert the configured destination address from text to an IP
    // address. Networking is intentionally not started on failure so a
    // typo in the configuration cannot cause a half-initialized library.
    IPAddress destination;

    if (!destination.fromString(config.host))
    {
        Serial.println("Telemetry: invalid host address");
        return false;
    }

    // Start the connection. Completion is observed non-blocking in
    // update(); nothing here waits for the link.
    wifiSsid = config.ssid;
    wifiPassword = config.password;

    WiFi.begin(wifiSsid, wifiPassword);
    lastWifiAttempt = millis();

    stream.begin(destination, config.streamerPort);
    commands.begin(config.commandPort);
    logs.begin(destination, config.logPort);

    initialized = true;
    return true;
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
    stream.update();
    commands.update();
    logs.update();
}
