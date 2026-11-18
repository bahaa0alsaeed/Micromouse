/**
 * @file Logger.h
 *
 * @brief Buffers textual log messages and transmits them over UDP.
 *
 * Part of the Telemetry stack; applications interact with this service
 * through Telemetry::logs. Messages are buffered at logging time and
 * transmitted in packets built by update().
 */

#pragma once

#include <cstdint>
#include <deque>
#include <string>
#include "IPAddress.h"
#include "WiFiUdp.h"


/**
 * @brief Severity levels for log messages.
 *
 * The levels form the conventional hierarchy used for filtering:
 * setting the level to X accepts messages of level X and of every
 * more severe level.
 */
enum class LogLevel
{
    Off = 0,        ///< Rejects all messages.
    Error = 1,      ///< Accepts error messages only.
    Warning = 2,    ///< Accepts warnings and errors.
    Info = 3,       ///< Accepts info, warnings and errors (default).
    Debug = 4       ///< Accepts all messages.
};


/**
 * @brief Collects textual log messages and transmits them over UDP.
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
     * @brief Sets the minimum severity level for accepted messages.
     *
     * Messages with the selected severity or higher severity are accepted.
     * LogLevel::Off disables all logging.
     *
     * @param level Minimum accepted severity level. Default: LogLevel::Info.
     */
    void setLevel(LogLevel level);

    /**
     * @brief Sets the minimum time interval between transmissions.
     *
     * When the buffered size is below SEND_THRESHOLD, transmission
     * is triggered after this interval has elapsed since the last
     * successful transmission. Reaching SEND_THRESHOLD triggers
     * transmission independently of this interval.
     *
     * @param ms Transmission interval in milliseconds. Zero is ignored.
     *           Default: 100 ms.
     */
    void setUpdateInterval(uint32_t ms);

    /**
     * @brief Sets the maximum UDP payload size, in bytes.
     *
     * The value must be between MAX_RECORD_SIZE and MAX_PACKET_SIZE.
     * Values outside this range are ignored. Zero restores the default
     * size (MAX_PACKET_SIZE).
     *
     * @param bytes Maximum UDP payload size in bytes.
     */
    void setMaxPacketSize(uint16_t bytes);

    /**
     * @brief Requests immediate transmission on the next update() call,
     * regardless of the interval and the buffered size.
     *
     * The request stays active until the entire buffer has been
     * transmitted, so remaining records are sent on subsequent
     * update() calls without waiting for the interval.
     *
     * At most one packet is transmitted per update() call.
     * Transmission still occurs exclusively inside update();
     * calling flush() never accesses the network directly.
     */
    void flush();

    /**
     * @brief Buffers an informational message.
     *
     * The message is prefixed with "[INFO] " and terminated with a
     * newline before being stored.
     *
     * @param message Message text, without the level prefix.
     * @return true if the message was accepted and buffered.
     *         Returns false if it was filtered out, exceeds
     *         MAX_RECORD_SIZE after formatting, or would exceed
     *         BUFFER_CAPACITY. A successful return does not imply
     *         transmission.
     */
    bool info(const std::string& message);

    /**
     * @brief Buffers a warning message with the "[WARNING]" prefix.
     *
     * @param message Message text, without the level prefix.
     * @return true if the message was accepted and buffered; false
     *         if it was filtered out or could not fit in the buffer.
     */
    bool warning(const std::string& message);

    /**
     * @brief Buffers an error message with the "[ERROR]" prefix.
     *
     * @param message Message text, without the level prefix.
     * @return true if the message was accepted and buffered; false
     *         if it was filtered out or could not fit in the buffer.
     */
    bool error(const std::string& message);

    /**
     * @brief Buffers a debug message with the "[DEBUG]" prefix.
     *
     * @param message Message text, without the level prefix.
     * @return true if the message was accepted and buffered; false
     *         if it was filtered out or could not fit in the buffer.
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

    // TODO: Replace with a fixed-capacity circular buffer to reduce
    /// dynamic allocation and improve memory predictability.
    std::deque<std::string> m_buffer;

    // Total size of the buffered records, prefix and newline included.
    size_t m_bufferedSize = 0;

    // Minimum accepted log severity.
    LogLevel m_level = LogLevel::Info;
    // Maximum UDP payload size update() may build, in bytes.
    uint16_t m_maxPacketSize = MAX_PACKET_SIZE;
    // Transmission interval in milliseconds.
    uint32_t m_updateInterval = 100;

    // Destination address for outgoing log packets.
    IPAddress m_destinationIP;
    // Destination UDP port. Zero means not configured.
    uint16_t m_destinationPort = 0;
    // UDP socket used to transmit log packets.
    WiFiUDP m_udp;

    // millis() timestamp of the last successful transmission.
    uint32_t m_lastTransmit = 0;
    // Set by flush(); cleared by transmit() once the buffer drains.
    bool m_flushRequested = false;


    /**
     * @brief Formats, filters and buffers a message. Shared by the four
     * public logging functions.
     *
     * @param level Severity of the message.
     * @param message Message text, without any level prefix.
     * @return true when the formatted record was accepted and stored
     *         in the buffer, false when it was rejected by the
     *         configured level, when it exceeds MAX_RECORD_SIZE, or
     *         when storing it would exceed BUFFER_CAPACITY.
     */
    bool log(LogLevel level, const std::string& message);

    /**
     * @brief Builds and sends one UDP packet from buffered records.
     *
     * The transmission scheduling conditions are handled by update().
     * This function handles packet construction and transmission.
     *
     * Records are removed from the buffer only after the complete UDP
     * send operation reports success. Failed sends leave the records
     * buffered for subsequent retry attempts.
     */
    void transmit();

    /**
     * @brief Configures the destination for outgoing UDP log messages.
     *
     * @param destination Destination IP address. Must not be 0.0.0.0.
     * @param port Destination UDP port. Must not be zero.
     *
     * @return true if the destination was successfully configured,
     *         false if the IP address or port is invalid.
     */
    bool begin(const IPAddress& destination, uint16_t port);


    /**
     * @brief Transmits buffered messages when transmission is due.
     *
     * Transmission is due when the buffered size reaches
     * SEND_THRESHOLD, when the configured interval has elapsed since
     * the last successful transmission, or when flush() was called.
     *
     * At most one packet is sent per call. The packet contains as many
     * complete records from the front of the FIFO as fit; records are
     * never split or truncated, and leftovers stay buffered for the
     * next call. Records are removed from the buffer only after the
     * UDP send reports success, so failed transmissions are retried
     * automatically. Never sends an empty packet.
     *
     * Called periodically by Telemetry::update().
     */
    void update(); // TODO: Send multiple packages at once.
};
