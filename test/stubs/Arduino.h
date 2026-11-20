/**
 * @file Arduino.h
 *
 * @brief Native stub of the Arduino core for host-side Telemetry tests.
 *
 * Provides a deterministic millisecond clock and a no-op Serial object,
 * covering only the APIs used by the Telemetry library. This header is
 * on the include path of the native test environment only and is never
 * part of the ESP32 firmware build.
 */

#ifndef Arduino_h
#define Arduino_h

#include <cstdint>

/// Deterministic fake millisecond clock shared by all translation units.
inline uint32_t g_fakeMillis = 0;

/// Returns the fake millisecond clock.
inline uint32_t millis()
{
    return g_fakeMillis;
}

/// Returns the fake microsecond clock.
inline uint32_t micros()
{
    return g_fakeMillis * 1000u;
}

/// Advances the fake clock instead of blocking.
inline void delay(uint32_t ms)
{
    g_fakeMillis += ms;
}

/// Accepts the delay without consuming time.
inline void delayMicroseconds(uint32_t)
{
}

/**
 * @brief No-op replacement for the Arduino Serial object.
 *
 * Every output operation is discarded so production logging calls do not
 * interfere with test output. The method set mirrors the subset that
 * Arduino sketches commonly use.
 */
class FakeSerialClass
{
public:
    static void begin(unsigned long) {}
    static void end() {}
    static void flush() {}

    template <typename... Args>
    static void printf(const char*, Args&&...)
    {
    }

    template <typename T>
    static void print(const T&)
    {
    }

    template <typename T>
    static void println(const T&)
    {
    }

    static void println() {}

    static int available() { return 0; }
    static int read() { return -1; }
};

/// Global Serial instance used by the stubbed Arduino headers.
inline FakeSerialClass Serial;

#endif  // Arduino_h
