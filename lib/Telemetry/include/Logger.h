#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <string>
#include "IPAddress.h"
#include "WiFiUdp.h"


/**
 * Severity levels for log messages.
 *
 * The levels form the conventional hierarchy used for filtering:
 * setting the level to X accepts messages of level X and of every
 * more severe level.
 */
enum class LogLevel
{
    Off = 0,    // Rejects every message.
    Error,      // Accepts error only.
    Warning,    // Accepts error + warning.
    Info,       // Accepts error + warning + info (default).
    Debug       // Accepts everything.
};


/**
 * Collects textual log messages and transmits them over UDP.
 *
 * Messages are logged through info(), warning(), error() and debug().
 * Each accepted message is stored in a FIFO buffer as a complete record
 * of the form "[LEVEL] message\n"; filtering by LogLevel happens at
 * logging time. The logging functions never touch the network: they only
 * buffer. All transmission happens in update(), which builds a single
 * UDP packet out of whole records from the front of the buffer.
 *
 * The service performs no Wi-Fi management: it is ticked by Telemetry
 * only while the connection is up, and transmits only after Telemetry
 * has provided a destination through its private begin().
 *
 * Accepting a message (the logging functions returning true) means the
 * record was buffered. It does not imply the message was transmitted or
 * received; UDP delivery is not acknowledged.
 */
class Logger
{
public:

    /**
     * Sets the filtering level.
     *
     * Messages less severe than the configured level are rejected by the
     * logging functions. LogLevel::Off rejects everything.
     */
    void setLevel(LogLevel level);

    /**
     * Sets the transmission interval in milliseconds.
     *
     * When buffered data is smaller than SEND_THRESHOLD, update()
     * transmits it once this interval has elapsed since the last
     * successful transmission. Zero is ignored. Default: 100 ms.
     */
    void setUpdateInterval(uint32_t ms);

    /**
     * Sets the maximum UDP payload size update() may build, in bytes.
     *
     * The value is clamped to the hard limit MAX_PACKET_SIZE (1200) and
     * can never make Logger exceed it. Zero resets it to the default.
     */
    void setMaxPacketSize(uint16_t bytes);

    /**
     * Requests immediate transmission on the next update() call,
     * regardless of the interval and the buffered size.
     *
     * The request stays active until the entire buffer has been
     * transmitted, so remaining records are sent on subsequent
     * update() calls without waiting for the interval.
     *
     * Still at most one packet per update() call, and still only
     * inside update(); calling this never touches the network directly.
     */
    void flush();

    /**
     * Buffers an informational message ("[INFO] ...").
     *
     * Returns true when the message was accepted and stored in the
     * buffer, false when it was rejected by the configured level, when
     * the formatted record exceeds MAX_RECORD_SIZE, or when storing it
     * would exceed BUFFER_CAPACITY. True does not mean transmitted.
     */
    bool info(const std::string& message);

    /**
     * Buffers a warning message ("[WARNING] ..."). See info().
     */
    bool warning(const std::string& message);

    /**
     * Buffers an error message ("[ERROR] ..."). See info().
     */
    bool error(const std::string& message);

    /**
     * Buffers a debug message ("[DEBUG] ..."). See info().
     */
    bool debug(const std::string& message);


    // Explicitly non-copyable: the services own UDP sockets and state.
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    friend class Telemetry;

    Logger() = default;

    // Maximum UDP payload size Logger is allowed to transmit in a
    // single packet. Kept separate from MAX_RECORD_SIZE on purpose.
    static constexpr size_t MAX_PACKET_SIZE = 1300;

    // Buffered size at which transmission becomes due. A trigger, not
    // a capacity: messages are still accepted past this point.
    static constexpr size_t SEND_THRESHOLD = 1100;

    // Maximum total size of the stored records. A record that would
    // overflow it is rejected; stored messages are never dropped.
    static constexpr size_t BUFFER_CAPACITY = 1300;

    // Maximum size of one complete formatted record, prefix and newline
    // included. Larger records are rejected entirely, never truncated.
    static constexpr size_t MAX_RECORD_SIZE = 1100;

    // Formats, filters and buffers a message. Shared by the four
    // public logging functions.
    bool log(LogLevel level, const std::string& message);

    // Sends one packet built from whole buffered records. All
    // transmission conditions are checked by update(); this only sends.
    void transmit();

    // TODO: Replace deque with Circular Buffer.
    // FIFO log buffer. Oldest records at the front, transmitted first.
    std::deque<std::string> m_buffer;

    // Total size of the buffered records, prefix and newline included.
    size_t m_bufferedSize = 0;

    LogLevel m_level = LogLevel::Info;

    IPAddress m_destinationIP;
    uint16_t m_destinationPort = 0;    // 0 means not configured: nothing is sent.

    uint32_t m_updateInterval = 100;
    uint32_t m_lastTransmit = 0;

    uint16_t m_maxPacketSize = MAX_PACKET_SIZE;
    bool m_flushRequested = false;

    WiFiUDP m_udp;


    // Called once by Telemetry::begin() with the resolved destination.
    void begin(const IPAddress& destination, uint16_t port);


    /**
     * Transmits buffered messages when transmission is due.
     *
     * Transmission is due when the buffered size reaches
     * SEND_THRESHOLD, when the configured interval has elapsed since
     * the last successful transmission, or when flush() was called.
     *
     * At most ONE packet is sent per call. The packet contains as many
     * complete records from the front of the FIFO as fit; records are
     * never split or truncated, and leftovers stay buffered for the
     * next call. Records are removed from the buffer only after the
     * UDP send reports success, so failed transmissions are retried
     * automatically. Never sends an empty packet.
     *
     * Call repeatedly from the main loop (Telemetry::update() does).
     */
    void update(); // TODO: Send multiple packages at once.
};
