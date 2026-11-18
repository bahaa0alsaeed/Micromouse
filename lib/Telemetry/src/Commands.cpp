/**
 * @file Commands.cpp
 *
 * @brief Implementation of the Commands service.
 *
 * Parses incoming "name,value1,..." frames completely before
 * dispatching them; malformed frames are discarded, never aborting.
 */

#include "Commands.h"

#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <utility>

namespace
{
// Maximum size of a received UDP frame that will be parsed; larger
// frames are drained and discarded.
constexpr size_t MAX_COMMAND_PACKET_SIZE = 128;

/**
 * @brief Parse one decimal floating-point field.
 *
 * Leading and trailing spaces/tabs are trimmed. Only plain decimal
 * notation is accepted: "nan", "inf" and hexadecimal forms are
 * rejected, as is any value whose magnitude exceeds the range of
 * float or that does not parse completely.
 *
 * @param begin First character of the field.
 * @param end One past the last character of the field.
 * @param result Receives the parsed value on success; untouched on
 *               failure.
 * @return true when the field holds a finite float value, false
 *         otherwise.
 */
bool parseField(const char* begin, const char* end, float& result)
{
    while (begin < end && (*begin == ' ' || *begin == '\t'))
        ++begin;

    while (end > begin && (end[-1] == ' ' || end[-1] == '\t'))
        --end;

    if (begin == end)
        return false;

    // Allow decimal notation only; reject nan, inf and hexadecimal forms.
    for (const char* p = begin; p < end; ++p)
    {
        const char c = *p;

        if (!((c >= '0' && c <= '9') ||
              c == '.' || c == 'e' || c == 'E' ||
              c == '+' || c == '-'))
            return false;
    }

    errno = 0;
    char* parseEnd = nullptr;
    const double value = std::strtod(begin, &parseEnd);

    // Reject partial parses, range errors and non-finite results.
    if (parseEnd != end || errno == ERANGE || !std::isfinite(value))
        return false;

    // Guard the float narrowing: the double parsed fine but may still
    // overflow when cast to float.
    if (std::abs(value) > std::numeric_limits<float>::max())
        return false;

    result = static_cast<float>(value);
    return true;
}

/**
 * @brief Parse the complete command before dispatching it.
 *
 * Trailing CR/LF characters are ignored. The text before the first
 * comma (or the whole frame when no comma is present) becomes the
 * command name; the remaining comma-separated fields are parsed as
 * numeric values. The frame is rejected as a whole when it is empty,
 * when the name is empty, when any value field does not parse, or
 * when it carries more than Command::MAX_VALUES values.
 *
 * @param frame Raw received frame, possibly with trailing newlines.
 * @param cmd Receives the parsed name, values and valueCount on
 *            success.
 * @return true when the frame parsed completely, false when it is
 *         malformed.
 */
bool parseCommand(const std::string& frame, Command& cmd)
{
    // Ignore trailing CR/LF characters.
    size_t length = frame.size();

    while (length > 0 &&
           (frame[length - 1] == '\r' || frame[length - 1] == '\n'))
        --length;

    if (length == 0)
        return false;

    const char* data = frame.data();
    const size_t firstComma = frame.find(',', 0);

    // The name ends at the first comma inside the frame, or at the
    // frame end when there is no comma (or it lies beyond the frame).
    const size_t nameEnd =
        firstComma == std::string::npos || firstComma >= length
            ? length
            : firstComma;

    // An empty name rejects the whole frame.
    if (nameEnd == 0)
        return false;

    cmd.name.assign(data, nameEnd);
    cmd.valueCount = 0;

    // A command without numeric arguments is valid.
    if (nameEnd == length)
        return true;

    // Parse the comma-separated value fields one by one.
    size_t start = nameEnd + 1;

    while (start <= length)
    {
        size_t end = start;

        while (end < length && data[end] != ',')
            ++end;

        // More values than Command::MAX_VALUES reject the frame.
        if (cmd.valueCount >= Command::MAX_VALUES)
            return false;

        if (!parseField(data + start, data + end,
                        cmd.values[cmd.valueCount]))
            return false;

        ++cmd.valueCount;

        if (end == length)
            break;

        start = end + 1;
    }

    return true;
}
}


bool Commands::on(const std::string& name, CommandHandler handler)
{
    if (name.empty() ||
        name.find_first_of(",=\n\r") != std::string::npos ||
        !handler)
    {
        return false;
    }

    m_handlers.push_back({name, std::move(handler)});
    return true;
}

bool Commands::on(const std::string& name, void (*handler)(const Command&))
{
    if (handler == nullptr)
        return false;

    return on(name, CommandHandler(handler));
}

bool Commands::begin(const uint16_t port)
{
    if (port == 0)
        return false;

    if (!m_udp.begin(port))
        return false;

    m_listenPort = port;
    return true;
}

void Commands::parseAndDispatch(const std::string& frame) const
{
    Command cmd;

    // Malformed frames are silently discarded: no handler is invoked.
    if (!parseCommand(frame, cmd))
        return;

    // Every handler registered under the command name is invoked, in
    // registration order.
    for (const auto& entry : m_handlers)
    {
        if (entry.name == cmd.name)
            entry.handler(cmd);
    }
}

void Commands::update()
{
    if (m_listenPort == 0)
        return;

    const int packetSize = m_udp.parsePacket();

    if (packetSize <= 0)
        return;

    // Frames larger than the maximum are drained and discarded.
    if (packetSize > static_cast<int>(MAX_COMMAND_PACKET_SIZE))
    {
        while (m_udp.available())
            m_udp.read();

        return;
    }

    m_rxBuffer.resize(static_cast<size_t>(packetSize));

    const int len = m_udp.read(
        reinterpret_cast<uint8_t*>(m_rxBuffer.data()),
        packetSize);

    // An incomplete read discards the frame without dispatching.
    if (len != packetSize)
        return;

    parseAndDispatch(m_rxBuffer);
}
