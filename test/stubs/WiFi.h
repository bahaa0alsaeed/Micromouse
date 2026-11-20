/**
 * @file WiFi.h
 *
 * @brief Controllable fake Wi-Fi driver for native Telemetry tests.
 *
 * Tracks connection attempts and credentials, and exposes a settable
 * connection status so tests can simulate disconnects and reconnects
 * without networking hardware. Native test environment only.
 */

#ifndef WiFi_h
#define WiFi_h

#include <cstdint>
#include <string>

#include "IPAddress.h"

/// Arduino-compatible Wi-Fi status codes used by the library.
enum wl_status_t : uint8_t
{
    WL_IDLE_STATUS = 0,
    WL_NO_SSID_AVAIL = 1,
    WL_SCAN_COMPLETED = 2,
    WL_CONNECTED = 3,
    WL_CONNECT_FAILED = 4,
    WL_CONNECTION_LOST = 5,
    WL_DISCONNECTED = 6
};

/**
 * @brief Fake Wi-Fi driver state.
 *
 * Every call to begin() is counted and its credentials recorded, so
 * tests can assert on connection and reconnection behavior. The
 * connection status is set directly by the test.
 */
class FakeWiFiClass
{
public:
    /**
     * @brief Records a connection attempt.
     *
     * @param ssid Network name; may be null.
     * @param password Network password; may be null.
     * @return The currently configured status (attempts are not
     *         asynchronous in the fake).
     */
    wl_status_t begin(const char* ssid, const char* password = nullptr)
    {
        ++beginCount;
        lastSsid = ssid != nullptr ? ssid : "";
        lastPassword = password != nullptr ? password : "";
        return wlStatus;
    }

    /// Returns the current fake connection status.
    wl_status_t status() const
    {
        return wlStatus;
    }

    /// Returns the fake local IP address.
    IPAddress localIP() const
    {
        return localIPAddress;
    }

    /// Sets the fake connection status.
    void setStatus(wl_status_t status)
    {
        wlStatus = status;
    }

    /// Restores the default fake state.
    void reset()
    {
        wlStatus = WL_CONNECTED;
        beginCount = 0;
        lastSsid.clear();
        lastPassword.clear();
        localIPAddress = IPAddress(192, 168, 1, 42);
    }

    /// Current fake connection status (defaults to connected).
    wl_status_t wlStatus = WL_CONNECTED;

    /// Number of begin() calls since the last reset.
    int beginCount = 0;

    /// SSID passed to the most recent begin() call.
    std::string lastSsid;

    /// Password passed to the most recent begin() call.
    std::string lastPassword;

    /// Fake local address reported by localIP().
    IPAddress localIPAddress{192, 168, 1, 42};
};

/// Global fake Wi-Fi driver used by the stubbed Arduino environment.
inline FakeWiFiClass WiFi;

#endif  // WiFi_h
