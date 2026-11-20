/**
 * @file WiFiUdp.h
 *
 * @brief In-memory UDP simulation for native Telemetry tests.
 *
 * All fake socket instances share a single state object, so tests can
 * inject incoming datagrams by local port without touching the private
 * sockets of the Telemetry services, and inspect every datagram the
 * services transmit.
 *
 * Receive side:
 * - Datagrams are queued per local port.
 * - parsePacket() moves the oldest datagram of the socket's bound port
 *   into the socket and reports its size.
 * - read() never crosses a datagram boundary; available() reports the
 *   unread bytes of the current datagram.
 *
 * Transmit side:
 * - beginPacket()/write()/endPacket() build a datagram which is
 *   recorded in sentDatagrams() when endPacket() succeeds.
 * - Failures can be injected at each stage (begin, partial write,
 *   end) to exercise the services' retry logic.
 *
 * Native test environment only.
 */

#ifndef _WIFIUDP_H_
#define _WIFIUDP_H_

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <deque>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "IPAddress.h"

/**
 * @brief One datagram captured from a fake UDP socket.
 */
struct FakeUdpDatagram
{
    /// Destination address used for beginPacket().
    IPAddress destination;

    /// Destination port used for beginPacket().
    uint16_t port = 0;

    /// Complete payload handed to the socket.
    std::string payload;
};

/**
 * @brief Shared state of the fake UDP environment.
 *
 * A single instance is shared by every WiFiUDP object so tests can
 * route datagrams by port and inspect all transmitted traffic.
 */
struct FakeUdpState
{
    /// Pending incoming datagrams, keyed by local port.
    std::map<uint16_t, std::deque<std::string>> receiveQueues;

    /// Datagrams successfully transmitted by any fake socket.
    std::vector<FakeUdpDatagram> sentDatagrams;

    /// Local ports passed to begin(), in call order.
    std::vector<uint16_t> boundPorts;

    /// When true, the next beginPacket() call fails and clears the flag.
    bool failNextBeginPacket = false;

    /// When true, the next endPacket() call fails and clears the flag.
    bool failNextEndPacket = false;

    /// When non-zero, the next write() is truncated to this many bytes,
    /// then the limit is cleared. Zero disables the simulation.
    std::size_t partialWriteLimit = 0;

    /**
     * @brief Queues an incoming datagram for a local port.
     *
     * @param localPort Local port the datagram is addressed to.
     * @param frame Datagram payload.
     */
    void inject(const uint16_t localPort, const std::string& frame)
    {
        receiveQueues[localPort].push_back(frame);
    }

    /// Clears queues, captured datagrams, bound ports and failure flags.
    void reset()
    {
        receiveQueues.clear();
        sentDatagrams.clear();
        boundPorts.clear();
        failNextBeginPacket = false;
        failNextEndPacket = false;
        partialWriteLimit = 0;
    }
};

/// Global fake UDP state shared by all sockets.
inline FakeUdpState g_fakeUdp;

/**
 * @brief Arduino-compatible UDP socket backed by FakeUdpState.
 *
 * Only the operations used by the Telemetry library and its tests are
 * implemented; semantics follow the ESP32 Arduino WiFiUDP class.
 */
class WiFiUDP
{
public:
    /**
     * @brief Binds the socket to a local port for reception.
     *
     * @param port Local UDP port.
     * @return 1 on success, mirroring WiFiUDP::begin().
     */
    int begin(uint16_t port)
    {
        m_localPort = port;
        g_fakeUdp.boundPorts.push_back(port);
        return 1;
    }

    /// Unbinds the socket.
    void stop()
    {
        m_localPort = 0;
    }

    /**
     * @brief Starts a datagram destined for a specific address.
     *
     * @param destination Destination IPv4 address.
     * @param port Destination UDP port.
     * @return 1 on success, 0 when a failure was injected.
     */
    int beginPacket(const IPAddress& destination, uint16_t port)
    {
        if (g_fakeUdp.failNextBeginPacket)
        {
            g_fakeUdp.failNextBeginPacket = false;
            m_txActive = false;
            return 0;
        }

        m_txDestination = destination;
        m_txPort = port;
        m_txPayload.clear();
        m_txActive = true;
        return 1;
    }

    /**
     * @brief Starts a datagram destined for a host name/address.
     *
     * @param host Dotted-quad destination address.
     * @param port Destination UDP port.
     * @return 1 on success, 0 on invalid host or injected failure.
     */
    int beginPacket(const char* host, uint16_t port)
    {
        IPAddress destination;

        if (!destination.fromString(host))
            return 0;

        return beginPacket(destination, port);
    }

    /**
     * @brief Appends bytes to the datagram being built.
     *
     * @param buffer Source bytes.
     * @param size Number of bytes to append.
     * @return Number of bytes accepted; less than size when a partial
     *         write was injected.
     */
    std::size_t write(const uint8_t* buffer, std::size_t size)
    {
        if (!m_txActive)
            return 0;

        std::size_t count = size;

        if (g_fakeUdp.partialWriteLimit != 0 &&
            count > g_fakeUdp.partialWriteLimit)
        {
            count = g_fakeUdp.partialWriteLimit;
            g_fakeUdp.partialWriteLimit = 0;
        }

        m_txPayload.append(reinterpret_cast<const char*>(buffer), count);
        return count;
    }

    /**
     * @brief Appends a single byte to the datagram being built.
     *
     * @param byte Byte to append.
     * @return 1 on success, 0 on failure.
     */
    std::size_t write(uint8_t byte)
    {
        return write(&byte, 1);
    }

    /**
     * @brief Completes and records the datagram being built.
     *
     * @return 1 when the datagram was recorded, 0 when no packet was
     *         active or an end failure was injected.
     */
    int endPacket()
    {
        if (!m_txActive)
            return 0;

        m_txActive = false;

        if (g_fakeUdp.failNextEndPacket)
        {
            g_fakeUdp.failNextEndPacket = false;
            return 0;
        }

        g_fakeUdp.sentDatagrams.push_back(
            {m_txDestination, m_txPort, m_txPayload});

        return 1;
    }

    /**
     * @brief Checks for a queued datagram on the socket's local port.
     *
     * @return Size of the dequeued datagram, or 0 when none is pending.
     */
    int parsePacket()
    {
        auto queue = g_fakeUdp.receiveQueues.find(m_localPort);

        if (queue == g_fakeUdp.receiveQueues.end() ||
            queue->second.empty())
        {
            m_rxCurrent.clear();
            m_rxOffset = 0;
            return 0;
        }

        m_rxCurrent = std::move(queue->second.front());
        queue->second.pop_front();
        m_rxOffset = 0;

        return static_cast<int>(m_rxCurrent.size());
    }

    /**
     * @brief Returns the unread bytes of the current datagram.
     *
     * @return Number of bytes still available, never crossing into the
     *         next datagram.
     */
    int available() const
    {
        return static_cast<int>(m_rxCurrent.size() - m_rxOffset);
    }

    /**
     * @brief Reads one byte from the current datagram.
     *
     * @return Byte value, or -1 when the datagram is exhausted.
     */
    int read()
    {
        if (available() <= 0)
            return -1;

        return static_cast<uint8_t>(m_rxCurrent[m_rxOffset++]);
    }

    /**
     * @brief Reads up to length bytes from the current datagram.
     *
     * @param buffer Destination buffer.
     * @param length Maximum bytes to read.
     * @return Number of bytes read; never crosses a datagram boundary.
     */
    int read(uint8_t* buffer, std::size_t length)
    {
        const std::size_t remaining = m_rxCurrent.size() - m_rxOffset;
        const std::size_t count = std::min(length, remaining);

        std::memcpy(buffer, m_rxCurrent.data() + m_rxOffset, count);
        m_rxOffset += count;

        return static_cast<int>(count);
    }

    /**
     * @brief Reads up to length bytes into a char buffer.
     *
     * @param buffer Destination buffer.
     * @param length Maximum bytes to read.
     * @return Number of bytes read.
     */
    int read(char* buffer, std::size_t length)
    {
        return read(reinterpret_cast<uint8_t*>(buffer), length);
    }

    /// Restores the per-socket state (queues are reset globally).
    void reset()
    {
        m_localPort = 0;
        m_rxCurrent.clear();
        m_rxOffset = 0;
        m_txDestination = IPAddress();
        m_txPort = 0;
        m_txPayload.clear();
        m_txActive = false;
    }

private:
    /// Local port this socket listens on.
    uint16_t m_localPort = 0;

    /// Current incoming datagram.
    std::string m_rxCurrent;

    /// Read offset inside the current datagram.
    std::size_t m_rxOffset = 0;

    /// Destination of the datagram being built.
    IPAddress m_txDestination;

    /// Destination port of the datagram being built.
    uint16_t m_txPort = 0;

    /// Payload of the datagram being built.
    std::string m_txPayload;

    /// True between beginPacket() and endPacket().
    bool m_txActive = false;
};

#endif  // _WIFIUDP_H_
