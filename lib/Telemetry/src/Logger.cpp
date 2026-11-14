#include "Logger.h"

// #include <utility>

#include <Arduino.h>


void Logger::setLevel(const LogLevel level)
{
    m_level = level;
}

void Logger::setUpdateInterval(const uint32_t ms)
{
    if (ms > 0)
        m_updateInterval = ms;
}

void Logger::setMaxPacketSize(const uint16_t bytes)
{
    // Clamp to the hard limit: the configured value can never make
    // Logger exceed MAX_PACKET_SIZE. Zero restores the default.
    if (bytes == 0 || bytes > MAX_PACKET_SIZE)
        m_maxPacketSize = MAX_PACKET_SIZE;
    else
        m_maxPacketSize = bytes;
}

void Logger::flush()
{
    // Only meaningful with buffered records: the request stays active
    // until the entire buffer has been transmitted (cleared by
    // transmit() after the buffer drains).
    if (!m_buffer.empty())
        m_flushRequested = true;
}


bool Logger::info(const std::string& message)
{
    return log(LogLevel::Info, message);
}

bool Logger::warning(const std::string& message)
{
    return log(LogLevel::Warning, message);
}

bool Logger::error(const std::string& message)
{
    return log(LogLevel::Error, message);
}

bool Logger::debug(const std::string& message)
{
    return log(LogLevel::Debug, message);
}


// Formats, filters and buffers a message.
bool Logger::log(const LogLevel level, const std::string& message)
{
    // Filter by the configured level. Off rejects everything; otherwise
    // the conventional hierarchy applies: level X accepts messages of
    // level X and of every more severe level (lower enum value).
    if (level == LogLevel::Off || m_level == LogLevel::Off || level > m_level)
        return false;

    // Indexed by (level - 1): Error, Warning, Info, Debug.
    static constexpr const char* PREFIXES[] =
    {
        "[ERROR] ", "[WARNING] ", "[INFO] ", "[DEBUG] "
    };


    std::string record = PREFIXES[static_cast<size_t>(level) - 1];
    record += message;
    record += '\n';

    // A record larger than the maximum record size is rejected
    // entirely: it is never truncated or split.
    if (record.size() > MAX_RECORD_SIZE)
        return false;

    // A record that would overflow the buffer capacity is rejected.
    // Previously stored messages are never dropped to make room.
    // m_bufferedSize never exceeds BUFFER_CAPACITY, so the subtraction
    // cannot underflow: the remaining space is always non-negative.
    if (record.size() > BUFFER_CAPACITY - m_bufferedSize)
        return false;

    m_buffer.push_back(std::move(record));
    m_bufferedSize += m_buffer.back().size();
    return true;
}

// Sends one packet built from whole buffered records. Called only by
// update(), which has already checked every transmission condition.
void Logger::transmit()
{
    // Never transmit without a configured destination or without
    // anything to send (empty packets are never sent).
    if (m_destinationPort == 0 || m_buffer.empty())
        return;

    // Build one packet out of complete records from the front of the
    // FIFO. A record that would not fit stays buffered for a later
    // call; records are never split between packets.
    std::string packet;
    size_t recordCount = 0;

    while (recordCount < m_buffer.size())
    {
        const std::string& record = m_buffer[recordCount];

        if (packet.size() + record.size() > m_maxPacketSize)
            break;

        packet += record;
        recordCount++;
    }

    if (packet.empty())
        return;

    // Attempt the send. write() and endPacket() are handled separately:
    // endPacket() is always called after a successful beginPacket() so
    // the socket is never left with a half-assembled packet, even when
    // write() wrote fewer bytes than expected.
    if (!m_udp.beginPacket(m_destinationIP, m_destinationPort))
        return;

    const size_t written = m_udp.write(
        reinterpret_cast<const uint8_t*>(packet.data()), packet.size());

    const int endResult = m_udp.endPacket();

    // The buffer is only modified when the complete send operation
    // succeeded (full packet written and endPacket() confirmed), so a
    // failure keeps every record intact for the next update() call.
    if (written != packet.size() || endResult != 1)
        return;

    // Success: drop the transmitted records.
    for (size_t i = 0; i < recordCount; i++)
    {
        m_bufferedSize -= m_buffer.front().size();
        m_buffer.pop_front();
    }

    // Timestamp the successful transmission immediately.
    m_lastTransmit = millis();

    // A flush request stays active until the entire buffer is empty, so
    // remaining records are sent by the following update() calls
    // without waiting for the interval.
    if (m_buffer.empty())
        m_flushRequested = false;
}

void Logger::update()
{
    // Transmission requires a configured destination and buffered data.
    // Empty packets are never sent.
    if (m_destinationPort == 0 || m_buffer.empty())
        return;

    const uint32_t now = millis();

    // Transmission is due when the buffered size reaches the threshold,
    // when the configured interval has elapsed (rollover-safe unsigned
    // comparison), or when flush() was requested.
    const bool due = m_bufferedSize >= SEND_THRESHOLD
                  || now - m_lastTransmit >= m_updateInterval
                  || m_flushRequested;

    if (!due)
        return;

    transmit();
}


void Logger::begin(const IPAddress& destination, const uint16_t port)
{
    m_destinationIP = destination;
    m_destinationPort = port;
}
