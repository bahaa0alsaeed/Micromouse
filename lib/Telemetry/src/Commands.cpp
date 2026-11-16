// #include "Commands.h"
//
// #include "CommandParser.h"
//
// namespace
// {
// // Packets larger than this cannot form a valid command frame (a command
// // name plus at most Command::MAX_VALUES numeric values). Oversized packets
// // are dropped before any receive memory is touched so that a flood of
// // large datagrams cannot pressure the heap.
// constexpr size_t MAX_COMMAND_PACKET_SIZE = 128;
// }
//
// void Commands::on(const std::string& name, CommandHandler handler)
// {
//     handlers.push_back({name, std::move(handler)});
// }
//
// void Commands::on(const std::string& name, void (*handler)(const Command&))
// {
//     // Convert the function pointer to the common handler type so both
//     // registration overloads use the same underlying implementation.
//     on(name, CommandHandler(handler));
// }
//
// void Commands::begin(const uint16_t port)
// {
//     listenPort = port;
//     bool result = udp.begin(port);
//     Serial.printf("[Commands] UDP begin: %s\n",
//                   result ? "OK" : "FAILED");
// }
//
// void Commands::update()
// {
//     // A port of zero indicates that the receiver has not been initialized.
//     if (listenPort == 0)
//         return;
//
//
//     int packetSize = udp.parsePacket();
//
//
//     // No UDP packet is currently waiting.
//     if (packetSize <= 0)
//         return;
//
//     // Drop packets that cannot possibly contain a valid command frame
//     // before resizing the receive buffer, keeping memory use bounded.
//     if (packetSize > static_cast<int>(MAX_COMMAND_PACKET_SIZE))
//     {
//         Serial.printf("[Commands] Dropped oversized packet (%d bytes)\n",
//                       packetSize);
//         return;
//     }
//
//     // Resize the reusable receive buffer to exactly match the incoming
//     // packet before reading its contents.
//     rxBuffer.assign(static_cast<size_t>(packetSize), '\0');
//
//     const int len = udp.read(reinterpret_cast<uint8_t*>(&rxBuffer[0]), packetSize);
//
//     // Ignore the frame if no data was actually read.
//     if (len <= 0)
//         return;
//
//     // Keep only the bytes that were successfully received.
//     rxBuffer.resize(static_cast<size_t>(len));
//
//     parseAndDispatch(rxBuffer);
// }
//
// void Commands::parseAndDispatch(const std::string& frame) const
// {
//     Command cmd;
//
//     // Parse the complete frame first. The parser either accepts every
//     // field or rejects the frame, so a partially parsed command can
//     // never reach a handler.
//     if (!CommandParser::parse(frame,
//                               cmd.name,
//                               cmd.values.data(),
//                               Command::MAX_VALUES,
//                               cmd.valueCount))
//     {
//         return;
//     }
//
//     for (const auto& entry : handlers)
//     {
//         if (entry.name == cmd.name)
//             entry.handler(cmd);
//     }
// }




#include "Commands.h"

#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <utility>

namespace
{
constexpr size_t MAX_COMMAND_PACKET_SIZE = 128;

// Parse one decimal floating-point field.
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

    if (parseEnd != end || errno == ERANGE || !std::isfinite(value))
        return false;

    if (std::abs(value) > std::numeric_limits<float>::max())
        return false;

    result = static_cast<float>(value);
    return true;
}

// Parse the complete command before dispatching it.
bool parseCommand(const std::string& frame, Command& cmd)
{
    size_t length = frame.size();

    while (length > 0 &&
           (frame[length - 1] == '\r' || frame[length - 1] == '\n'))
        --length;

    if (length == 0)
        return false;

    const char* data = frame.data();
    const size_t firstComma = frame.find(',', 0);

    const size_t nameEnd =
        firstComma == std::string::npos || firstComma >= length
            ? length
            : firstComma;

    if (nameEnd == 0)
        return false;

    cmd.name.assign(data, nameEnd);
    cmd.valueCount = 0;

    // A command without numeric arguments is valid.
    if (nameEnd == length)
        return true;

    size_t start = nameEnd + 1;

    while (start <= length)
    {
        size_t end = start;

        while (end < length && data[end] != ',')
            ++end;

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

void Commands::on(const std::string& name, CommandHandler handler)
{
    handlers.push_back({name, std::move(handler)});
}

void Commands::on(const std::string& name, void (*handler)(const Command&))
{
    on(name, CommandHandler(handler));
}

void Commands::begin(const uint16_t port)
{
    listenPort = port;
    udp.begin(port);
}

void Commands::update()
{
    if (listenPort == 0)
        return;

    const int packetSize = udp.parsePacket();

    if (packetSize <= 0)
        return;

    if (packetSize > static_cast<int>(MAX_COMMAND_PACKET_SIZE))
    {
        while (udp.available())
            udp.read();

        return;
    }

    rxBuffer.resize(static_cast<size_t>(packetSize));

    const int len = udp.read(
        reinterpret_cast<uint8_t*>(rxBuffer.data()),
        packetSize);

    if (len != packetSize)
        return;

    parseAndDispatch(rxBuffer);
}

void Commands::parseAndDispatch(const std::string& frame) const
{
    Command cmd;

    if (!parseCommand(frame, cmd))
        return;

    for (const auto& entry : handlers)
    {
        if (entry.name == cmd.name)
            entry.handler(cmd);
    }
}
