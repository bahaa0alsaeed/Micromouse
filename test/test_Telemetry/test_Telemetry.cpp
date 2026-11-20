/**
 * @file test_Telemetry.cpp
 *
 * @brief Native Unity tests for the Telemetry library.
 *
 * The tests run the production Telemetry, Streamer, Commands and Logger
 * implementations on the host against the fake Arduino/Wi-Fi/UDP headers
 * in test/stubs. Every test drives the library through its public API
 * only; incoming datagrams are injected by port and outgoing traffic is
 * inspected through the shared fake UDP state.
 */

#include <unity.h>

// Include the native stubs from this file so the test also resolves
// correctly when an IDE parses it with the ESP32 compile configuration.
// The stubs reuse the include guards of the real Arduino/Wi-Fi headers,
// preventing those headers from being pulled in behind them.
#include "../stubs/Arduino.h"
#include "../stubs/IPAddress.h"
#include "../stubs/WiFi.h"
#include "../stubs/WiFiUdp.h"
#include <Telemetry.h>

#include <cmath>
#include <limits>
#include <memory>
#include <string>
#include <vector>

// ============================================================
// Test constants and environment helpers
// ============================================================

namespace
{
constexpr uint16_t STREAMER_PORT = 9000;
constexpr uint16_t COMMAND_PORT = 9001;
constexpr uint16_t LOG_PORT = 9002;

constexpr const char* TEST_HOST = "192.168.1.100";
constexpr const char* TEST_SSID = "TestSSID";
constexpr const char* TEST_PASSWORD = "TestPassword";

/// Telemetry instance under test, recreated for every test.
std::unique_ptr<Telemetry> g_telemetry;

/// Number of calls recorded by the main test handler.
int g_recordedCalls = 0;

/// Last command received by the main test handler.
Command g_recordedCommand;

/// Number of calls recorded by the secondary test handler.
int g_secondHandlerCalls = 0;

/// Speed value shared by the integration test handler.
float g_integrationSpeed = 0.0f;

/// Records the received command.
void recordCommand(const Command& command)
{
    ++g_recordedCalls;
    g_recordedCommand = command;
}

/// Records a call without capturing the command.
void recordSecondCommand(const Command& command)
{
    (void)command;
    ++g_secondHandlerCalls;
}

/// Integration test handler that stores a single-value speed command.
void integrationSpeedHandler(const Command& command)
{
    if (command.valueCount == 1)
        g_integrationSpeed = command.values[0];
}

/// Returns the default valid test configuration.
TelemetryConfig testConfig()
{
    TelemetryConfig config;

    config.ssid = TEST_SSID;
    config.password = TEST_PASSWORD;
    config.host = TEST_HOST;
    config.streamerPort = STREAMER_PORT;
    config.commandPort = COMMAND_PORT;
    config.logPort = LOG_PORT;

    return config;
}

/// Clears all fake hardware state and test recorder state.
void clearAll()
{
    g_fakeMillis = 0;
    g_fakeUdp.reset();
    WiFi.reset();

    g_recordedCalls = 0;
    g_recordedCommand = Command{};
    g_secondHandlerCalls = 0;
    g_integrationSpeed = 0.0f;
}

/// Clears fake hardware state and creates a fresh Telemetry instance.
void resetEnvironment()
{
    clearAll();
    g_telemetry = std::make_unique<Telemetry>();
}

/// Advances the fake millisecond clock.
void advanceTime(uint32_t ms)
{
    g_fakeMillis += ms;
}

/// Queues an incoming datagram for a local port.
void injectFrame(uint16_t port, const std::string& frame)
{
    g_fakeUdp.inject(port, frame);
}

/// Returns the most recently transmitted datagram, or nullptr.
const FakeUdpDatagram* lastSentPacket()
{
    if (g_fakeUdp.sentDatagrams.empty())
        return nullptr;

    return &g_fakeUdp.sentDatagrams.back();
}

/// Returns all datagrams transmitted to the given destination port.
std::vector<FakeUdpDatagram> packetsTo(uint16_t port)
{
    std::vector<FakeUdpDatagram> packets;

    for (const auto& datagram : g_fakeUdp.sentDatagrams)
    {
        if (datagram.port == port)
            packets.push_back(datagram);
    }

    return packets;
}

/// Returns the number of datagrams transmitted to the given port.
int datagramCount(uint16_t port)
{
    return static_cast<int>(packetsTo(port).size());
}

/// Returns the number of datagrams to a port whose payload contains token.
int datagramCountContaining(uint16_t port, const std::string& token)
{
    int count = 0;

    for (const auto& datagram : packetsTo(port))
    {
        if (datagram.payload.find(token) != std::string::npos)
            ++count;
    }

    return count;
}
}

namespace
{
    // ============================================================
    // A. Logger tests
    // ============================================================

    /**
     * @brief Default log level accepts info/warning/error but not debug.
     */
    void test_logger_default_level_filters_debug()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        TEST_ASSERT_TRUE(telemetry.logs.error("e"));
        TEST_ASSERT_TRUE(telemetry.logs.warning("w"));
        TEST_ASSERT_TRUE(telemetry.logs.info("i"));
        TEST_ASSERT_FALSE(telemetry.logs.debug("d"));
    }

    /**
     * @brief Debug level accepts every severity.
     */
    void test_logger_debug_level_accepts_all()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));
        telemetry.logs.setLevel(LogLevel::Debug);

        TEST_ASSERT_TRUE(telemetry.logs.error("e"));
        TEST_ASSERT_TRUE(telemetry.logs.warning("w"));
        TEST_ASSERT_TRUE(telemetry.logs.info("i"));
        TEST_ASSERT_TRUE(telemetry.logs.debug("d"));
    }

    /**
     * @brief Error level rejects warning, info and debug messages.
     */
    void test_logger_error_level_filters_lower_severities()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));
        telemetry.logs.setLevel(LogLevel::Error);

        TEST_ASSERT_TRUE(telemetry.logs.error("e"));
        TEST_ASSERT_FALSE(telemetry.logs.warning("w"));
        TEST_ASSERT_FALSE(telemetry.logs.info("i"));
        TEST_ASSERT_FALSE(telemetry.logs.debug("d"));
    }

    /**
     * @brief Off level rejects every message.
     */
    void test_logger_off_level_rejects_all()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));
        telemetry.logs.setLevel(LogLevel::Off);

        TEST_ASSERT_FALSE(telemetry.logs.error("e"));
        TEST_ASSERT_FALSE(telemetry.logs.warning("w"));
        TEST_ASSERT_FALSE(telemetry.logs.info("i"));
        TEST_ASSERT_FALSE(telemetry.logs.debug("d"));
    }

    /**
     * @brief Records are formatted as "[LEVEL] message\n" in FIFO order.
     */
    void test_logger_formats_prefix_and_newline()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));
        telemetry.logs.setLevel(LogLevel::Debug);

        TEST_ASSERT_TRUE(telemetry.logs.error("e"));
        TEST_ASSERT_TRUE(telemetry.logs.warning("w"));
        TEST_ASSERT_TRUE(telemetry.logs.info("i"));
        TEST_ASSERT_TRUE(telemetry.logs.debug("d"));

        telemetry.logs.flush();
        telemetry.update();

        const auto packets = packetsTo(LOG_PORT);
        TEST_ASSERT_EQUAL_INT(1, static_cast<int>(packets.size()));
        TEST_ASSERT_EQUAL_STRING(
            "[ERROR] e\n[WARNING] w\n[INFO] i\n[DEBUG] d\n",
            packets[0].payload.c_str());
    }

    /**
     * @brief flush() transmits buffered records on the next update().
     */
    void test_logger_flush_sends_buffered_records()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        TEST_ASSERT_TRUE(telemetry.logs.info("hello"));

        // At t = 0 the interval has not elapsed and no flush was requested.
        telemetry.update();
        TEST_ASSERT_EQUAL_INT(0, datagramCount(LOG_PORT));

        telemetry.logs.flush();
        telemetry.update();

        TEST_ASSERT_EQUAL_INT(1, datagramCount(LOG_PORT));
        TEST_ASSERT_EQUAL_STRING(
            "[INFO] hello\n",
            lastSentPacket()->payload.c_str());

    }

    /**
     * @brief Transmission is gated by the configured interval.
     */
    void test_logger_interval_gating()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        TEST_ASSERT_TRUE(telemetry.logs.info("first"));

        telemetry.update();
        TEST_ASSERT_EQUAL_INT(0, datagramCount(LOG_PORT));

        advanceTime(99);
        telemetry.update();
        TEST_ASSERT_EQUAL_INT(0, datagramCount(LOG_PORT));

        advanceTime(1);
        telemetry.update();
        TEST_ASSERT_EQUAL_INT(1, datagramCount(LOG_PORT));
    }

    /**
     * @brief A record exceeding MAX_RECORD_SIZE (1100 bytes) is rejected.
     */
    void test_logger_rejects_record_over_max_size()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        // "[INFO] " (7) + message + '\n' must be <= 1100 bytes.
        const std::string boundary(1092, 'a');
        const std::string tooLong(1093, 'a');

        TEST_ASSERT_TRUE(telemetry.logs.info(boundary));
        TEST_ASSERT_FALSE(telemetry.logs.info(tooLong));

        telemetry.logs.flush();
        telemetry.update();

        const auto packets = packetsTo(LOG_PORT);
        TEST_ASSERT_EQUAL_INT(1, static_cast<int>(packets.size()));
        TEST_ASSERT_EQUAL_INT(1100, static_cast<int>(packets[0].payload.size()));
    }

    /**
     * @brief Empty messages are accepted and formatted normally.
     */
    void test_logger_accepts_empty_message()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        TEST_ASSERT_TRUE(telemetry.logs.info(""));
        telemetry.logs.flush();
        telemetry.update();

        TEST_ASSERT_EQUAL_STRING(
            "[INFO] \n",
            lastSentPacket()->payload.c_str());
    }

    /**
     * @brief Messages are delivered in FIFO order.
     */
    void test_logger_preserves_message_order()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        TEST_ASSERT_TRUE(telemetry.logs.error("first"));
        TEST_ASSERT_TRUE(telemetry.logs.warning("second"));
        TEST_ASSERT_TRUE(telemetry.logs.info("third"));

        telemetry.logs.flush();
        telemetry.update();

        TEST_ASSERT_EQUAL_STRING(
            "[ERROR] first\n[WARNING] second\n[INFO] third\n",
            lastSentPacket()->payload.c_str());
    }

    /**
     * @brief A message that would overflow the buffer is rejected while
     *        previously buffered messages are preserved.
     */
    void test_logger_rejects_buffer_overflow_preserving_messages()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        const std::string chunk(700, 'b');  // 708-byte record.

        TEST_ASSERT_TRUE(telemetry.logs.info(chunk));
        // 708 + 708 > 1300: the second record must be rejected.
        TEST_ASSERT_FALSE(telemetry.logs.info(chunk));

        telemetry.logs.flush();
        telemetry.update();

        const auto packets = packetsTo(LOG_PORT);
        TEST_ASSERT_EQUAL_INT(1, static_cast<int>(packets.size()));

        const std::string expected = "[INFO] " + chunk + "\n";
        TEST_ASSERT_EQUAL_INT(
            static_cast<int>(expected.size()),
            static_cast<int>(packets[0].payload.size()));
        TEST_ASSERT_EQUAL_STRING(expected.c_str(), packets[0].payload.c_str());
    }

    /**
     * @brief At most one packet is transmitted per update() call.
     */
    void test_logger_sends_one_packet_per_update()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        TEST_ASSERT_TRUE(telemetry.logs.info("one"));
        TEST_ASSERT_TRUE(telemetry.logs.info("two"));

        telemetry.logs.flush();
        telemetry.update();

        TEST_ASSERT_EQUAL_INT(1, datagramCount(LOG_PORT));

        telemetry.update();
        TEST_ASSERT_EQUAL_INT(1, datagramCount(LOG_PORT));
    }

    /**
     * @brief Reaching SEND_THRESHOLD triggers transmission before the
     *        interval elapses.
     */
    void test_logger_buffered_threshold_triggers_transmission()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        // Two 553-byte records total 1106 bytes >= 1100 threshold.
        const std::string chunk(545, 't');
        TEST_ASSERT_TRUE(telemetry.logs.info(chunk));
        TEST_ASSERT_TRUE(telemetry.logs.info(chunk));

        telemetry.update();

        const auto packets = packetsTo(LOG_PORT);
        TEST_ASSERT_EQUAL_INT(1, static_cast<int>(packets.size()));
        TEST_ASSERT_EQUAL_INT(1106, static_cast<int>(packets[0].payload.size()));
    }

    /**
     * @brief A beginPacket() failure keeps records buffered for retry.
     */
    void test_logger_retries_after_begin_packet_failure()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        TEST_ASSERT_TRUE(telemetry.logs.info("retry"));
        telemetry.logs.flush();

        g_fakeUdp.failNextBeginPacket = true;
        telemetry.update();
        TEST_ASSERT_EQUAL_INT(0, datagramCount(LOG_PORT));

        telemetry.update();
        TEST_ASSERT_EQUAL_INT(1, datagramCount(LOG_PORT));
        TEST_ASSERT_EQUAL_STRING(
            "[INFO] retry\n",
            lastSentPacket()->payload.c_str());
    }

    /**
     * @brief A partial write() keeps records buffered and retries in full.
     */
    void test_logger_retries_after_partial_write()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        TEST_ASSERT_TRUE(telemetry.logs.info("retry"));
        telemetry.logs.flush();

        g_fakeUdp.partialWriteLimit = 4;
        telemetry.update();

        auto packets = packetsTo(LOG_PORT);
        TEST_ASSERT_EQUAL_INT(1, static_cast<int>(packets.size()));
        TEST_ASSERT_EQUAL_INT(4, static_cast<int>(packets[0].payload.size()));

        telemetry.update();

        packets = packetsTo(LOG_PORT);
        TEST_ASSERT_EQUAL_INT(2, static_cast<int>(packets.size()));
        TEST_ASSERT_EQUAL_STRING(
            "[INFO] retry\n",
            packets[1].payload.c_str());
    }

    /**
     * @brief An endPacket() failure keeps records buffered for retry.
     */
    void test_logger_retries_after_end_packet_failure()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        TEST_ASSERT_TRUE(telemetry.logs.info("retry"));
        telemetry.logs.flush();

        g_fakeUdp.failNextEndPacket = true;
        telemetry.update();
        TEST_ASSERT_EQUAL_INT(0, datagramCount(LOG_PORT));

        telemetry.update();
        TEST_ASSERT_EQUAL_INT(1, datagramCount(LOG_PORT));
        TEST_ASSERT_EQUAL_STRING(
            "[INFO] retry\n",
            lastSentPacket()->payload.c_str());
    }

    /**
     * @brief No empty datagram is ever transmitted.
     */
    void test_logger_never_sends_empty_packet()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        telemetry.update();
        advanceTime(10000);
        telemetry.update();

        TEST_ASSERT_EQUAL_INT(0, datagramCount(LOG_PORT));
    }

    // ============================================================
    // B. Streamer tests
    // ============================================================

    /**
     * @brief Valid registrations are accepted in both groups.
     */
    void test_streamer_accepts_valid_registrations()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        float fastValue = 1.0f;
        float slowValue = 2.0f;

        TEST_ASSERT_TRUE(telemetry.stream.addFast("fast", &fastValue));
        TEST_ASSERT_TRUE(telemetry.stream.addSlow("slow", &slowValue));
    }

    /**
     * @brief Invalid names and null pointers are rejected.
     */
    void test_streamer_rejects_invalid_registrations()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        float value = 1.0f;

        TEST_ASSERT_FALSE(telemetry.stream.addFast("", &value));
        TEST_ASSERT_FALSE(telemetry.stream.addFast("a,b", &value));
        TEST_ASSERT_FALSE(telemetry.stream.addFast("a=b", &value));
        TEST_ASSERT_FALSE(telemetry.stream.addFast("a\nb", &value));
        TEST_ASSERT_FALSE(telemetry.stream.addFast("a\rb", &value));
        TEST_ASSERT_FALSE(telemetry.stream.addFast("null", nullptr));

        TEST_ASSERT_FALSE(telemetry.stream.addSlow("", &value));
        TEST_ASSERT_FALSE(telemetry.stream.addSlow("a,b", &value));
        TEST_ASSERT_FALSE(telemetry.stream.addSlow("a=b", &value));
        TEST_ASSERT_FALSE(telemetry.stream.addSlow("null", nullptr));
    }

    /**
     * @brief Payloads are formatted as "name=value,...\n" in registration
     *        order and sent to the configured destination.
     */
    void test_streamer_formats_payload()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        float speed = 12.5f;
        float temperature = -3.25f;

        TEST_ASSERT_TRUE(telemetry.stream.addFast("speed", &speed));
        TEST_ASSERT_TRUE(telemetry.stream.addFast("temperature", &temperature));

        advanceTime(120);
        telemetry.update();

        const auto packets = packetsTo(STREAMER_PORT);
        TEST_ASSERT_EQUAL_INT(1, static_cast<int>(packets.size()));

        const std::string expected =
            "speed=" + std::to_string(speed) +
            ",temperature=" + std::to_string(temperature) + "\n";

        TEST_ASSERT_EQUAL_STRING(expected.c_str(), packets[0].payload.c_str());
        TEST_ASSERT_TRUE(packets[0].destination == IPAddress(192, 168, 1, 100));
        TEST_ASSERT_EQUAL_UINT16(STREAMER_PORT, packets[0].port);
    }

    /**
     * @brief Values are read from the registered pointers at send time.
     */
    void test_streamer_reads_current_value_at_send_time()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        float speed = 1.0f;
        TEST_ASSERT_TRUE(telemetry.stream.addFast("speed", &speed));

        speed = 42.0f;

        advanceTime(120);
        telemetry.update();

        const std::string expected = "speed=" + std::to_string(speed) + "\n";
        TEST_ASSERT_EQUAL_STRING(expected.c_str(), lastSentPacket()->payload.c_str());
    }

    /**
     * @brief Duplicate variable names are allowed and both are streamed.
     */
    void test_streamer_allows_duplicate_names()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        float first = 1.0f;
        float second = 2.0f;

        TEST_ASSERT_TRUE(telemetry.stream.addFast("x", &first));
        TEST_ASSERT_TRUE(telemetry.stream.addFast("x", &second));

        advanceTime(120);
        telemetry.update();

        const std::string expected =
            "x=" + std::to_string(first) + ",x=" + std::to_string(second) + "\n";

        TEST_ASSERT_EQUAL_STRING(expected.c_str(), lastSentPacket()->payload.c_str());
    }

    /**
     * @brief The fast group respects its configured interval.
     */
    void test_streamer_fast_interval_gating()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        float value = 1.0f;
        TEST_ASSERT_TRUE(telemetry.stream.addFast("v", &value));

        advanceTime(119);
        telemetry.update();
        TEST_ASSERT_EQUAL_INT(0, datagramCount(STREAMER_PORT));

        advanceTime(1);
        telemetry.update();
        TEST_ASSERT_EQUAL_INT(1, datagramCount(STREAMER_PORT));

        advanceTime(119);
        telemetry.update();
        TEST_ASSERT_EQUAL_INT(1, datagramCount(STREAMER_PORT));

        advanceTime(1);
        telemetry.update();
        TEST_ASSERT_EQUAL_INT(2, datagramCount(STREAMER_PORT));
    }

    /**
     * @brief The slow group respects its configured interval.
     */
    void test_streamer_slow_interval_gating()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        float value = 1.0f;
        TEST_ASSERT_TRUE(telemetry.stream.addSlow("v", &value));

        advanceTime(499);
        telemetry.update();
        TEST_ASSERT_EQUAL_INT(0, datagramCount(STREAMER_PORT));

        advanceTime(1);
        telemetry.update();
        TEST_ASSERT_EQUAL_INT(1, datagramCount(STREAMER_PORT));
    }

    /**
     * @brief Fast and slow groups are scheduled independently.
     */
    void test_streamer_groups_are_independent()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        float fastValue = 1.0f;
        float slowValue = 2.0f;

        TEST_ASSERT_TRUE(telemetry.stream.addFast("fast", &fastValue));
        TEST_ASSERT_TRUE(telemetry.stream.addSlow("slow", &slowValue));

        advanceTime(120);
        telemetry.update();

        TEST_ASSERT_EQUAL_INT(1, datagramCountContaining(STREAMER_PORT, "fast="));
        TEST_ASSERT_EQUAL_INT(0, datagramCountContaining(STREAMER_PORT, "slow="));

        advanceTime(380);  // t = 500 ms.
        telemetry.update();

        TEST_ASSERT_EQUAL_INT(2, datagramCountContaining(STREAMER_PORT, "fast="));
        TEST_ASSERT_EQUAL_INT(1, datagramCountContaining(STREAMER_PORT, "slow="));
    }

    /**
     * @brief A zero interval is ignored and keeps the current interval.
     */
    void test_streamer_zero_interval_is_ignored()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        float value = 1.0f;
        TEST_ASSERT_TRUE(telemetry.stream.addFast("v", &value));

        telemetry.stream.setFastInterval(0);

        advanceTime(119);
        telemetry.update();
        TEST_ASSERT_EQUAL_INT(0, datagramCount(STREAMER_PORT));

        advanceTime(1);
        telemetry.update();
        TEST_ASSERT_EQUAL_INT(1, datagramCount(STREAMER_PORT));
    }

    /**
     * @brief Empty groups never transmit, even after a long time.
     */
    void test_streamer_empty_groups_send_nothing()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        advanceTime(10000);
        telemetry.update();

        TEST_ASSERT_EQUAL_INT(0, datagramCount(STREAMER_PORT));
    }

    /**
     * @brief Non-finite values are streamed using the standard float text
     *        representation ("nan"/"inf").
     */
    void test_streamer_non_finite_values()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        float notANumber = std::numeric_limits<float>::quiet_NaN();
        float infinity = std::numeric_limits<float>::infinity();

        TEST_ASSERT_TRUE(telemetry.stream.addFast("nan_value", &notANumber));
        TEST_ASSERT_TRUE(telemetry.stream.addFast("inf_value", &infinity));

        advanceTime(120);
        telemetry.update();

        const std::string expected =
            "nan_value=" + std::to_string(notANumber) +
            ",inf_value=" + std::to_string(infinity) + "\n";

        TEST_ASSERT_EQUAL_STRING(expected.c_str(), lastSentPacket()->payload.c_str());
    }

    /**
     * @brief Timer arithmetic stays correct across the 32-bit rollover.
     */
    void test_streamer_timer_rollover()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        float value = 1.0f;
        TEST_ASSERT_TRUE(telemetry.stream.addFast("v", &value));

        // Timestamp shortly before the clock wraps.
        g_fakeMillis = 0xFFFFFFF5u;  // UINT32_MAX - 10.
        telemetry.update();
        TEST_ASSERT_EQUAL_INT(1, datagramCount(STREAMER_PORT));

        // 141 ms after the previous send, computed across the wrap.
        g_fakeMillis = 130u;
        telemetry.update();
        TEST_ASSERT_EQUAL_INT(2, datagramCount(STREAMER_PORT));
    }

    // ============================================================
    // C. Commands tests
    // ============================================================

    /**
     * @brief A command without values is dispatched with valueCount 0.
     */
    void test_commands_dispatches_command_without_values()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));
        TEST_ASSERT_TRUE(telemetry.commands.on("stop", recordCommand));

        injectFrame(COMMAND_PORT, "stop");
        telemetry.update();

        TEST_ASSERT_EQUAL_INT(1, g_recordedCalls);
        TEST_ASSERT_EQUAL_STRING("stop", g_recordedCommand.name.c_str());
        TEST_ASSERT_EQUAL_UINT8(0, g_recordedCommand.valueCount);
    }

    /**
     * @brief A single numeric value is parsed and dispatched.
     */
    void test_commands_dispatches_single_value()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));
        TEST_ASSERT_TRUE(telemetry.commands.on("set_speed", recordCommand));

        injectFrame(COMMAND_PORT, "set_speed,200");
        telemetry.update();

        TEST_ASSERT_EQUAL_INT(1, g_recordedCalls);
        TEST_ASSERT_EQUAL_UINT8(1, g_recordedCommand.valueCount);
        TEST_ASSERT_EQUAL_FLOAT(200.0f, g_recordedCommand.values[0]);
    }

    /**
     * @brief Three numeric values (PID gains) are parsed and dispatched.
     */
    void test_commands_dispatches_three_values()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));
        TEST_ASSERT_TRUE(telemetry.commands.on("set_pid", recordCommand));

        injectFrame(COMMAND_PORT, "set_pid,1.2,0.05,0.8");
        telemetry.update();

        TEST_ASSERT_EQUAL_INT(1, g_recordedCalls);
        TEST_ASSERT_EQUAL_UINT8(3, g_recordedCommand.valueCount);
        TEST_ASSERT_FLOAT_WITHIN(1e-6f, 1.2f, g_recordedCommand.values[0]);
        TEST_ASSERT_FLOAT_WITHIN(1e-6f, 0.05f, g_recordedCommand.values[1]);
        TEST_ASSERT_FLOAT_WITHIN(1e-6f, 0.8f, g_recordedCommand.values[2]);
    }

    /**
     * @brief Negative values are parsed and dispatched.
     */
    void test_commands_dispatches_negative_values()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));
        TEST_ASSERT_TRUE(telemetry.commands.on("set_position", recordCommand));

        injectFrame(COMMAND_PORT, "set_position,-10,20");
        telemetry.update();

        TEST_ASSERT_EQUAL_INT(1, g_recordedCalls);
        TEST_ASSERT_EQUAL_UINT8(2, g_recordedCommand.valueCount);
        TEST_ASSERT_EQUAL_FLOAT(-10.0f, g_recordedCommand.values[0]);
        TEST_ASSERT_EQUAL_FLOAT(20.0f, g_recordedCommand.values[1]);
    }

    /**
     * @brief A frame with a trailing comma (empty value) is rejected.
     */
    void test_commands_rejects_trailing_comma()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));
        TEST_ASSERT_TRUE(telemetry.commands.on("set_speed", recordCommand));

        injectFrame(COMMAND_PORT, "set_speed,");
        telemetry.update();

        TEST_ASSERT_EQUAL_INT(0, g_recordedCalls);
    }

    /**
     * @brief An empty value field inside the frame is rejected.
     */
    void test_commands_rejects_empty_argument()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));
        TEST_ASSERT_TRUE(telemetry.commands.on("set_pid", recordCommand));

        injectFrame(COMMAND_PORT, "set_pid,1.2,,0.8");
        telemetry.update();

        TEST_ASSERT_EQUAL_INT(0, g_recordedCalls);
    }

    /**
     * @brief A non-numeric value is rejected.
     */
    void test_commands_rejects_non_numeric_value()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));
        TEST_ASSERT_TRUE(telemetry.commands.on("set_speed", recordCommand));

        injectFrame(COMMAND_PORT, "set_speed,abc");
        telemetry.update();

        TEST_ASSERT_EQUAL_INT(0, g_recordedCalls);
    }

    /**
     * @brief A partially numeric value is rejected.
     */
    void test_commands_rejects_partial_numeric_value()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));
        TEST_ASSERT_TRUE(telemetry.commands.on("set_speed", recordCommand));

        injectFrame(COMMAND_PORT, "set_speed,200abc");
        telemetry.update();

        TEST_ASSERT_EQUAL_INT(0, g_recordedCalls);
    }

    /**
     * @brief More than Command::MAX_VALUES values are rejected.
     */
    void test_commands_rejects_too_many_values()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));
        TEST_ASSERT_TRUE(telemetry.commands.on("set_pid", recordCommand));

        injectFrame(COMMAND_PORT, "set_pid,1,2,3,4,5,6,7,8,9");
        telemetry.update();

        TEST_ASSERT_EQUAL_INT(0, g_recordedCalls);
    }

    /**
     * @brief NaN, Infinity and out-of-float-range values are rejected.
     */
    void test_commands_rejects_non_finite_values()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));
        TEST_ASSERT_TRUE(telemetry.commands.on("set_speed", recordCommand));

        const char* invalidFrames[] = {
            "set_speed,nan",
            "set_speed,inf",
            "set_speed,-inf",
            "set_speed,infinity",
            "set_speed,0x1A",
            "set_speed,1e999",
            "set_speed,1e40",
        };

        for (const char* frame : invalidFrames)
        {
            g_recordedCalls = 0;
            injectFrame(COMMAND_PORT, frame);
            telemetry.update();
            TEST_ASSERT_EQUAL_INT(0, g_recordedCalls);
        }
    }

    /**
     * @brief Empty command names and empty frames are rejected.
     */
    void test_commands_rejects_empty_name_and_empty_frame()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));
        TEST_ASSERT_FALSE(telemetry.commands.on("", recordCommand));
        TEST_ASSERT_TRUE(telemetry.commands.on("stop", recordCommand));

        injectFrame(COMMAND_PORT, ",1,2");
        telemetry.update();
        TEST_ASSERT_EQUAL_INT(0, g_recordedCalls);

        // A frame that is empty after stripping CR/LF is rejected.
        injectFrame(COMMAND_PORT, "\r\n");
        telemetry.update();
        TEST_ASSERT_EQUAL_INT(0, g_recordedCalls);
    }

    /**
     * @brief Oversized datagrams are drained without dispatch and the
     *        receiver keeps working afterwards.
     */
    void test_commands_rejects_oversized_datagram_and_recovers()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));
        TEST_ASSERT_TRUE(telemetry.commands.on("stop", recordCommand));

        // MAX_COMMAND_PACKET_SIZE is 128 bytes; this frame has 129.
        const std::string oversized = "stop," + std::string(124, '0');
        TEST_ASSERT_EQUAL_INT(129, static_cast<int>(oversized.size()));

        injectFrame(COMMAND_PORT, oversized);
        telemetry.update();
        TEST_ASSERT_EQUAL_INT(0, g_recordedCalls);

        injectFrame(COMMAND_PORT, "stop");
        telemetry.update();
        TEST_ASSERT_EQUAL_INT(1, g_recordedCalls);
    }

    /**
     * @brief A datagram of exactly the maximum size is accepted.
     */
    void test_commands_accepts_max_size_datagram()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));
        TEST_ASSERT_TRUE(telemetry.commands.on("set_speed", recordCommand));

        // "set_speed," (10) + "0." (2) + 116 zeros == 128 bytes.
        const std::string frame = "set_speed,0." + std::string(116, '0');
        TEST_ASSERT_EQUAL_INT(128, static_cast<int>(frame.size()));

        injectFrame(COMMAND_PORT, frame);
        telemetry.update();

        TEST_ASSERT_EQUAL_INT(1, g_recordedCalls);
        TEST_ASSERT_EQUAL_UINT8(1, g_recordedCommand.valueCount);
        TEST_ASSERT_EQUAL_FLOAT(0.0f, g_recordedCommand.values[0]);
    }

    /**
     * @brief Multiple queued datagrams are processed one per update() and
     *        in arrival order.
     */
    void test_commands_processes_multiple_datagrams_in_order()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));
        TEST_ASSERT_TRUE(telemetry.commands.on("set_speed", recordCommand));

        injectFrame(COMMAND_PORT, "set_speed,1");
        injectFrame(COMMAND_PORT, "set_speed,2");
        injectFrame(COMMAND_PORT, "set_speed,3");

        telemetry.update();
        TEST_ASSERT_EQUAL_INT(1, g_recordedCalls);
        TEST_ASSERT_EQUAL_FLOAT(1.0f, g_recordedCommand.values[0]);

        telemetry.update();
        TEST_ASSERT_EQUAL_INT(2, g_recordedCalls);
        TEST_ASSERT_EQUAL_FLOAT(2.0f, g_recordedCommand.values[0]);

        telemetry.update();
        TEST_ASSERT_EQUAL_INT(3, g_recordedCalls);
        TEST_ASSERT_EQUAL_FLOAT(3.0f, g_recordedCommand.values[0]);

        telemetry.update();
        TEST_ASSERT_EQUAL_INT(3, g_recordedCalls);
    }

    /**
     * @brief A datagram is dispatched exactly once.
     */
    void test_commands_dispatches_exactly_once()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));
        TEST_ASSERT_TRUE(telemetry.commands.on("stop", recordCommand));

        injectFrame(COMMAND_PORT, "stop");
        telemetry.update();
        telemetry.update();
        telemetry.update();

        TEST_ASSERT_EQUAL_INT(1, g_recordedCalls);
    }

    /**
     * @brief Registering the same name twice invokes every handler.
     */
    void test_commands_duplicate_registration_invokes_all_handlers()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        TEST_ASSERT_TRUE(telemetry.commands.on("stop", recordCommand));
        TEST_ASSERT_TRUE(telemetry.commands.on("stop", recordSecondCommand));

        injectFrame(COMMAND_PORT, "stop");
        telemetry.update();

        TEST_ASSERT_EQUAL_INT(1, g_recordedCalls);
        TEST_ASSERT_EQUAL_INT(1, g_secondHandlerCalls);
    }

    /**
     * @brief Unknown command names are ignored.
     */
    void test_commands_unknown_command_is_ignored()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));
        TEST_ASSERT_TRUE(telemetry.commands.on("stop", recordCommand));

        injectFrame(COMMAND_PORT, "unknown,1,2");
        telemetry.update();

        TEST_ASSERT_EQUAL_INT(0, g_recordedCalls);
    }

    /**
     * @brief The receiver recovers after a malformed datagram.
     */
    void test_commands_recovers_after_malformed_datagram()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));
        TEST_ASSERT_TRUE(telemetry.commands.on("set_speed", recordCommand));

        injectFrame(COMMAND_PORT, "set_speed,abc");
        telemetry.update();
        TEST_ASSERT_EQUAL_INT(0, g_recordedCalls);

        injectFrame(COMMAND_PORT, "set_speed,5");
        telemetry.update();
        TEST_ASSERT_EQUAL_INT(1, g_recordedCalls);
        TEST_ASSERT_EQUAL_FLOAT(5.0f, g_recordedCommand.values[0]);
    }

    /**
     * @brief Consecutive datagrams do not leak values into each other.
     */
    void test_commands_no_stale_values_between_datagrams()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));
        TEST_ASSERT_TRUE(telemetry.commands.on("set_speed", recordCommand));
        TEST_ASSERT_TRUE(telemetry.commands.on("stop", recordCommand));

        injectFrame(COMMAND_PORT, "set_speed,200");
        telemetry.update();
        TEST_ASSERT_EQUAL_UINT8(1, g_recordedCommand.valueCount);

        injectFrame(COMMAND_PORT, "stop");
        telemetry.update();

        TEST_ASSERT_EQUAL_STRING("stop", g_recordedCommand.name.c_str());
        TEST_ASSERT_EQUAL_UINT8(0, g_recordedCommand.valueCount);
        TEST_ASSERT_EQUAL_FLOAT(0.0f, g_recordedCommand.values[0]);
    }

    /**
     * @brief Trailing CR/LF transport characters are ignored.
     */
    void test_commands_strips_crlf()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));
        TEST_ASSERT_TRUE(telemetry.commands.on("stop", recordCommand));

        injectFrame(COMMAND_PORT, "stop\r\n");
        telemetry.update();

        TEST_ASSERT_EQUAL_INT(1, g_recordedCalls);
        TEST_ASSERT_EQUAL_STRING("stop", g_recordedCommand.name.c_str());
    }

    /**
     * @brief Surrounding whitespace around numeric values is tolerated.
     */
    void test_commands_accepts_whitespace_around_value()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));
        TEST_ASSERT_TRUE(telemetry.commands.on("set_speed", recordCommand));

        injectFrame(COMMAND_PORT, "set_speed, 200 ");
        telemetry.update();

        TEST_ASSERT_EQUAL_INT(1, g_recordedCalls);
        TEST_ASSERT_EQUAL_FLOAT(200.0f, g_recordedCommand.values[0]);
    }

    /**
     * @brief Invalid handler registrations are rejected.
     */
    void test_commands_registration_validation()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        TEST_ASSERT_FALSE(telemetry.commands.on("", recordCommand));
        TEST_ASSERT_FALSE(telemetry.commands.on("bad,name", recordCommand));
        TEST_ASSERT_FALSE(telemetry.commands.on("bad=name", recordCommand));
        TEST_ASSERT_FALSE(telemetry.commands.on("bad\nname", recordCommand));
        TEST_ASSERT_FALSE(telemetry.commands.on("name", CommandHandler()));

        void (*nullHandler)(const Command&) = nullptr;
        TEST_ASSERT_FALSE(telemetry.commands.on("name", nullHandler));
    }

    // ============================================================
    // D. Telemetry facade tests
    // ============================================================

    /**
     * @brief A malformed destination address rejects initialization.
     */
    void test_telemetry_rejects_invalid_host()
    {
        resetEnvironment();

        TelemetryConfig config = testConfig();
        config.host = "not-an-address";

        TEST_ASSERT_FALSE(g_telemetry->begin(config));
        TEST_ASSERT_EQUAL_INT(0, WiFi.beginCount);
        TEST_ASSERT_TRUE(g_fakeUdp.boundPorts.empty());
    }

    /**
     * @brief A null destination address rejects initialization.
     */
    void test_telemetry_rejects_null_host()
    {
        resetEnvironment();

        TelemetryConfig config = testConfig();
        config.host = nullptr;

        TEST_ASSERT_FALSE(g_telemetry->begin(config));
        TEST_ASSERT_EQUAL_INT(0, WiFi.beginCount);
    }

    /**
     * @brief The unspecified address 0.0.0.0 rejects initialization.
     */
    void test_telemetry_rejects_unspecified_host()
    {
        resetEnvironment();

        TelemetryConfig config = testConfig();
        config.host = "0.0.0.0";

        TEST_ASSERT_FALSE(g_telemetry->begin(config));
        TEST_ASSERT_EQUAL_INT(0, WiFi.beginCount);
        TEST_ASSERT_TRUE(g_fakeUdp.boundPorts.empty());
    }

    /**
     * @brief A zero streamer port rejects initialization.
     */
    void test_telemetry_rejects_zero_streamer_port()
    {
        resetEnvironment();

        TelemetryConfig config = testConfig();
        config.streamerPort = 0;

        TEST_ASSERT_FALSE(g_telemetry->begin(config));
        TEST_ASSERT_EQUAL_INT(0, WiFi.beginCount);
    }

    /**
     * @brief A zero command port rejects initialization.
     */
    void test_telemetry_rejects_zero_command_port()
    {
        resetEnvironment();

        TelemetryConfig config = testConfig();
        config.commandPort = 0;

        TEST_ASSERT_FALSE(g_telemetry->begin(config));
        TEST_ASSERT_EQUAL_INT(0, WiFi.beginCount);
        TEST_ASSERT_TRUE(g_fakeUdp.boundPorts.empty());
    }

    /**
     * @brief A zero log port rejects initialization.
     */
    void test_telemetry_rejects_zero_log_port()
    {
        resetEnvironment();

        TelemetryConfig config = testConfig();
        config.logPort = 0;

        TEST_ASSERT_FALSE(g_telemetry->begin(config));
        TEST_ASSERT_EQUAL_INT(0, WiFi.beginCount);
    }

    /**
     * @brief Invalid initialization never starts Wi-Fi.
     */
    void test_telemetry_invalid_init_does_not_start_wifi()
    {
        resetEnvironment();

        TelemetryConfig config = testConfig();
        config.host = "";
        TEST_ASSERT_FALSE(g_telemetry->begin(config));
        TEST_ASSERT_EQUAL_INT(0, WiFi.beginCount);

        config = testConfig();
        config.streamerPort = 0;
        TEST_ASSERT_FALSE(g_telemetry->begin(config));
        TEST_ASSERT_EQUAL_INT(0, WiFi.beginCount);
    }

    /**
     * @brief Successful initialization starts Wi-Fi and binds the command
     *        port.
     */
    void test_telemetry_successful_init()
    {
        resetEnvironment();

        TEST_ASSERT_TRUE(g_telemetry->begin(testConfig()));

        TEST_ASSERT_EQUAL_INT(1, WiFi.beginCount);
        TEST_ASSERT_EQUAL_STRING(TEST_SSID, WiFi.lastSsid.c_str());
        TEST_ASSERT_EQUAL_STRING(TEST_PASSWORD, WiFi.lastPassword.c_str());

        TEST_ASSERT_EQUAL_INT(1, static_cast<int>(g_fakeUdp.boundPorts.size()));
        TEST_ASSERT_EQUAL_UINT16(COMMAND_PORT, g_fakeUdp.boundPorts[0]);
    }

    /**
     * @brief update() before begin() does nothing.
     */
    void test_telemetry_update_before_begin_does_nothing()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;

        TEST_ASSERT_TRUE(telemetry.commands.on("stop", recordCommand));

        injectFrame(COMMAND_PORT, "stop");
        TEST_ASSERT_TRUE(telemetry.logs.info("before"));
        telemetry.logs.flush();

        telemetry.update();

        TEST_ASSERT_EQUAL_INT(0, g_recordedCalls);
        TEST_ASSERT_EQUAL_INT(0, datagramCount(LOG_PORT));
        TEST_ASSERT_EQUAL_INT(0, WiFi.beginCount);
    }

    /**
     * @brief Services are not ticked while Wi-Fi is disconnected.
     */
    void test_telemetry_services_not_updated_while_disconnected()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        float value = 1.0f;
        TEST_ASSERT_TRUE(telemetry.stream.addFast("v", &value));
        TEST_ASSERT_TRUE(telemetry.commands.on("stop", recordCommand));
        TEST_ASSERT_TRUE(telemetry.logs.info("queued"));
        telemetry.logs.flush();

        WiFi.setStatus(WL_DISCONNECTED);

        injectFrame(COMMAND_PORT, "stop");
        advanceTime(1000);
        telemetry.update();

        TEST_ASSERT_EQUAL_INT(0, g_recordedCalls);
        TEST_ASSERT_EQUAL_INT(0, datagramCount(LOG_PORT));
        TEST_ASSERT_EQUAL_INT(0, datagramCount(STREAMER_PORT));
    }

    /**
     * @brief Services are ticked while Wi-Fi is connected.
     */
    void test_telemetry_services_updated_while_connected()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        TEST_ASSERT_TRUE(telemetry.commands.on("stop", recordCommand));
        TEST_ASSERT_TRUE(telemetry.logs.info("queued"));
        telemetry.logs.flush();

        injectFrame(COMMAND_PORT, "stop");
        telemetry.update();

        TEST_ASSERT_EQUAL_INT(1, g_recordedCalls);
        TEST_ASSERT_EQUAL_INT(1, datagramCount(LOG_PORT));
    }

    /**
     * @brief A reconnect is attempted once the retry interval elapses.
     */
    void test_telemetry_reconnects_after_interval()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));
        TEST_ASSERT_EQUAL_INT(1, WiFi.beginCount);

        WiFi.setStatus(WL_DISCONNECTED);

        advanceTime(10000);
        telemetry.update();
        TEST_ASSERT_EQUAL_INT(2, WiFi.beginCount);

        advanceTime(9999);
        telemetry.update();
        TEST_ASSERT_EQUAL_INT(2, WiFi.beginCount);

        advanceTime(1);
        telemetry.update();
        TEST_ASSERT_EQUAL_INT(3, WiFi.beginCount);
    }

    /**
     * @brief No reconnect attempt occurs before the retry interval expires.
     */
    void test_telemetry_no_premature_reconnect()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        WiFi.setStatus(WL_DISCONNECTED);

        advanceTime(9999);
        telemetry.update();

        TEST_ASSERT_EQUAL_INT(1, WiFi.beginCount);
    }

    /**
     * @brief Services resume when Wi-Fi reconnects.
     */
    void test_telemetry_resumes_services_after_reconnect()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        TEST_ASSERT_TRUE(telemetry.commands.on("stop", recordCommand));

        WiFi.setStatus(WL_DISCONNECTED);
        injectFrame(COMMAND_PORT, "stop");
        telemetry.update();
        TEST_ASSERT_EQUAL_INT(0, g_recordedCalls);

        WiFi.setStatus(WL_CONNECTED);
        telemetry.update();

        TEST_ASSERT_EQUAL_INT(1, g_recordedCalls);
    }

    /**
     * @brief Reconnecting before the retry interval does not cause an
     *        immediate new connection attempt.
     */
    void test_telemetry_reconnect_before_interval_expires()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        WiFi.setStatus(WL_DISCONNECTED);
        advanceTime(3000);

        WiFi.setStatus(WL_CONNECTED);
        telemetry.update();

        WiFi.setStatus(WL_DISCONNECTED);
        telemetry.update();
        TEST_ASSERT_EQUAL_INT(1, WiFi.beginCount);

        advanceTime(7000);
        telemetry.update();
        TEST_ASSERT_EQUAL_INT(2, WiFi.beginCount);
    }

    /**
     * @brief Repeated disconnect/reconnect cycles keep retrying.
     */
    void test_telemetry_repeated_disconnect_reconnect_cycles()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        int expectedAttempts = 1;

        for (int cycle = 0; cycle < 3; ++cycle)
        {
            WiFi.setStatus(WL_DISCONNECTED);
            advanceTime(10000);
            telemetry.update();

            ++expectedAttempts;
            TEST_ASSERT_EQUAL_INT(expectedAttempts, WiFi.beginCount);

            WiFi.setStatus(WL_CONNECTED);
            telemetry.update();
        }
    }

    /**
     * @brief Calling begin() repeatedly reconfigures the services and starts
     *        a new Wi-Fi attempt each time.
     */
    void test_telemetry_repeated_begin()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;

        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));
        advanceTime(500);
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        TEST_ASSERT_EQUAL_INT(2, WiFi.beginCount);
        TEST_ASSERT_EQUAL_INT(2, static_cast<int>(g_fakeUdp.boundPorts.size()));
        TEST_ASSERT_EQUAL_UINT16(COMMAND_PORT, g_fakeUdp.boundPorts[1]);
    }

    // ============================================================
    // E. Integration tests
    // ============================================================

    /**
     * @brief Command handling, streaming and logging work together.
     */
    void test_integration_command_updates_stream_and_log()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        TEST_ASSERT_TRUE(telemetry.stream.addFast("speed", &g_integrationSpeed));
        TEST_ASSERT_TRUE(
            telemetry.commands.on("set_speed", integrationSpeedHandler));
        TEST_ASSERT_TRUE(telemetry.logs.info("hello"));

        injectFrame(COMMAND_PORT, "set_speed,150");
        telemetry.logs.flush();
        telemetry.update();

        // The command handler updates the streamed variable.
        TEST_ASSERT_FLOAT_WITHIN(1e-6f, 150.0f, g_integrationSpeed);

        // The logger transmits during the same update().
        TEST_ASSERT_EQUAL_INT(1, datagramCount(LOG_PORT));
        TEST_ASSERT_EQUAL_STRING(
            "[INFO] hello\n",
            lastSentPacket()->payload.c_str());

        // The fast streamer is not due at t = 0.
        TEST_ASSERT_EQUAL_INT(0, datagramCount(STREAMER_PORT));

        advanceTime(120);
        telemetry.update();

        // The streamer now transmits the updated value.
        const auto packets = packetsTo(STREAMER_PORT);
        TEST_ASSERT_EQUAL_INT(1, static_cast<int>(packets.size()));

        const std::string expected =
            "speed=" + std::to_string(g_integrationSpeed) + "\n";
        TEST_ASSERT_EQUAL_STRING(expected.c_str(), packets[0].payload.c_str());
    }

    /**
     * @brief The services recover correctly after a Wi-Fi disconnection.
     */
    void test_integration_survives_wifi_disconnect()
    {
        resetEnvironment();
        Telemetry& telemetry = *g_telemetry;
        TEST_ASSERT_TRUE(telemetry.begin(testConfig()));

        TEST_ASSERT_TRUE(telemetry.stream.addFast("speed", &g_integrationSpeed));
        TEST_ASSERT_TRUE(
            telemetry.commands.on("set_speed", integrationSpeedHandler));

        TEST_ASSERT_TRUE(telemetry.logs.info("first"));
        telemetry.logs.flush();
        telemetry.update();
        TEST_ASSERT_EQUAL_INT(1, datagramCount(LOG_PORT));

        WiFi.setStatus(WL_DISCONNECTED);

        injectFrame(COMMAND_PORT, "set_speed,200");
        TEST_ASSERT_TRUE(telemetry.logs.info("second"));
        telemetry.logs.flush();

        advanceTime(1000);
        telemetry.update();

        // Disconnected: nothing is dispatched, streamed or logged.
        TEST_ASSERT_EQUAL_FLOAT(0.0f, g_integrationSpeed);
        TEST_ASSERT_EQUAL_INT(1, datagramCount(LOG_PORT));
        TEST_ASSERT_EQUAL_INT(0, datagramCount(STREAMER_PORT));

        WiFi.setStatus(WL_CONNECTED);
        telemetry.update();

        // Reconnected: queued command and log are processed.
        TEST_ASSERT_FLOAT_WITHIN(1e-6f, 200.0f, g_integrationSpeed);
        TEST_ASSERT_EQUAL_INT(2, datagramCount(LOG_PORT));
        TEST_ASSERT_EQUAL_STRING(
            "[INFO] second\n",
            lastSentPacket()->payload.c_str());

        // The first post-reconnect streamer tick happens before command
        // dispatch within the same update() (service order: stream,
        // commands, logs), so it still carries the previous value.
        advanceTime(120);
        telemetry.update();

        const auto packets = packetsTo(STREAMER_PORT);
        TEST_ASSERT_EQUAL_INT(2, static_cast<int>(packets.size()));

        const std::string expected =
            "speed=" + std::to_string(g_integrationSpeed) + "\n";
        TEST_ASSERT_EQUAL_STRING(expected.c_str(), packets[1].payload.c_str());

    }
}

// ============================================================
// Main
// ============================================================

int main()
{
    UNITY_BEGIN();

    // A. Logger
    RUN_TEST(test_logger_default_level_filters_debug);
    RUN_TEST(test_logger_debug_level_accepts_all);
    RUN_TEST(test_logger_error_level_filters_lower_severities);
    RUN_TEST(test_logger_off_level_rejects_all);
    RUN_TEST(test_logger_formats_prefix_and_newline);
    RUN_TEST(test_logger_flush_sends_buffered_records);
    RUN_TEST(test_logger_interval_gating);
    RUN_TEST(test_logger_rejects_record_over_max_size);
    RUN_TEST(test_logger_accepts_empty_message);
    RUN_TEST(test_logger_preserves_message_order);
    RUN_TEST(test_logger_rejects_buffer_overflow_preserving_messages);
    RUN_TEST(test_logger_sends_one_packet_per_update);
    RUN_TEST(test_logger_buffered_threshold_triggers_transmission);
    RUN_TEST(test_logger_retries_after_begin_packet_failure);
    RUN_TEST(test_logger_retries_after_partial_write);
    RUN_TEST(test_logger_retries_after_end_packet_failure);
    RUN_TEST(test_logger_never_sends_empty_packet);

    // B. Streamer
    RUN_TEST(test_streamer_accepts_valid_registrations);
    RUN_TEST(test_streamer_rejects_invalid_registrations);
    RUN_TEST(test_streamer_formats_payload);
    RUN_TEST(test_streamer_reads_current_value_at_send_time);
    RUN_TEST(test_streamer_allows_duplicate_names);
    RUN_TEST(test_streamer_fast_interval_gating);
    RUN_TEST(test_streamer_slow_interval_gating);
    RUN_TEST(test_streamer_groups_are_independent);
    RUN_TEST(test_streamer_zero_interval_is_ignored);
    RUN_TEST(test_streamer_empty_groups_send_nothing);
    RUN_TEST(test_streamer_non_finite_values);
    RUN_TEST(test_streamer_timer_rollover);

    // C. Commands
    RUN_TEST(test_commands_dispatches_command_without_values);
    RUN_TEST(test_commands_dispatches_single_value);
    RUN_TEST(test_commands_dispatches_three_values);
    RUN_TEST(test_commands_dispatches_negative_values);
    RUN_TEST(test_commands_rejects_trailing_comma);
    RUN_TEST(test_commands_rejects_empty_argument);
    RUN_TEST(test_commands_rejects_non_numeric_value);
    RUN_TEST(test_commands_rejects_partial_numeric_value);
    RUN_TEST(test_commands_rejects_too_many_values);
    RUN_TEST(test_commands_rejects_non_finite_values);
    RUN_TEST(test_commands_rejects_empty_name_and_empty_frame);
    RUN_TEST(test_commands_rejects_oversized_datagram_and_recovers);
    RUN_TEST(test_commands_accepts_max_size_datagram);
    RUN_TEST(test_commands_processes_multiple_datagrams_in_order);
    RUN_TEST(test_commands_dispatches_exactly_once);
    RUN_TEST(test_commands_duplicate_registration_invokes_all_handlers);
    RUN_TEST(test_commands_unknown_command_is_ignored);
    RUN_TEST(test_commands_recovers_after_malformed_datagram);
    RUN_TEST(test_commands_no_stale_values_between_datagrams);
    RUN_TEST(test_commands_strips_crlf);
    RUN_TEST(test_commands_accepts_whitespace_around_value);
    RUN_TEST(test_commands_registration_validation);

    // D. Telemetry facade
    RUN_TEST(test_telemetry_rejects_invalid_host);
    RUN_TEST(test_telemetry_rejects_null_host);
    RUN_TEST(test_telemetry_rejects_unspecified_host);
    RUN_TEST(test_telemetry_rejects_zero_streamer_port);
    RUN_TEST(test_telemetry_rejects_zero_command_port);
    RUN_TEST(test_telemetry_rejects_zero_log_port);
    RUN_TEST(test_telemetry_invalid_init_does_not_start_wifi);
    RUN_TEST(test_telemetry_successful_init);
    RUN_TEST(test_telemetry_update_before_begin_does_nothing);
    RUN_TEST(test_telemetry_services_not_updated_while_disconnected);
    RUN_TEST(test_telemetry_services_updated_while_connected);
    RUN_TEST(test_telemetry_reconnects_after_interval);
    RUN_TEST(test_telemetry_no_premature_reconnect);
    RUN_TEST(test_telemetry_resumes_services_after_reconnect);
    RUN_TEST(test_telemetry_reconnect_before_interval_expires);
    RUN_TEST(test_telemetry_repeated_disconnect_reconnect_cycles);
    RUN_TEST(test_telemetry_repeated_begin);

    // E. Integration
    RUN_TEST(test_integration_command_updates_stream_and_log);
    RUN_TEST(test_integration_survives_wifi_disconnect);

    return UNITY_END();
}
