#include "Commands.h"

#include <cerrno>

namespace
{
/**
 * Parses the numeric part of a command frame.
 *
 * Replaces the original std::stof call, which threw an uncaught
 * std::invalid_argument exception (aborting the ESP32) whenever the
 * value part was not a valid number, e.g. "set_speed,abc".
 *
 * The value must be fully numeric; trailing characters other than
 * spaces/tabs make the frame invalid.
 */
bool parseCommandValue(const std::string& text, float& out)
{
    const char* start = text.c_str();
    char* end = nullptr;

    errno = 0;
    const double value = strtod(start, &end);

    if (end == start || errno == ERANGE)
        return false;

    // Trailing spaces are tolerated, any other trailing character is not.
    while (*end == ' ' || *end == '\t')
        end++;

    if (*end != '\0')
        return false;

    out = static_cast<float>(value);
    return true;
}
}

void Commands::on(const std::string& name, CommandHandler handler)
{
    handlers.push_back({name, std::move(handler)});
}

void Commands::on(const std::string& name, void (*handler)(const Command&))
{
    // Convert the function pointer to the common handler type so both
    // registration overloads use the same underlying implementation.
    on(name, CommandHandler(handler));
}

void Commands::begin(const uint16_t port)
{
    listenPort = port;
    bool result = udp.begin(port);
    Serial.printf("[Commands] UDP begin: %s\n",
                  result ? "OK" : "FAILED");
}

void Commands::update()
{
    // A port of zero indicates that the receiver has not been initialized.
    if (listenPort == 0)
        return;


    int packetSize = udp.parsePacket();

    if (packetSize > 0)
    {
        Serial.printf(
            "[Commands] Packet received: %d bytes from %s:%u\n",
            packetSize,
            udp.remoteIP().toString().c_str(),
            udp.remotePort()
        );
    }

    // No UDP packet is currently waiting.
    if (packetSize <= 0)
        return;

    // Resize the reusable receive buffer to exactly match the incoming
    // packet before reading its contents.
    rxBuffer.assign(static_cast<size_t>(packetSize), '\0');

    const int len = udp.read(reinterpret_cast<uint8_t*>(&rxBuffer[0]), packetSize);

    // Ignore the frame if no data was actually read.
    if (len <= 0)
        return;

    // Keep only the bytes that were successfully received.
    rxBuffer.resize(static_cast<size_t>(len));

    parseAndDispatch(rxBuffer);
}

void Commands::parseAndDispatch(const std::string& frame) const {
    std::string content = frame;

    Serial.printf(
        "[DISPATCH] at %lu ms\n",
        millis()
    );

    // Remove trailing line-ending characters so that command matching is
    // performed against the actual command name/value rather than transport
    // formatting added to the received frame.
    while (!content.empty() && (content.back() == '\n' || content.back() == '\r'))
    {
        content.pop_back();
    }

    if (content.empty())
        return;

    Command cmd;

    const size_t comma = content.find(',');

    if (comma == std::string::npos)
    {
        // No comma means the entire frame is treated as the command name.
        cmd.name = content;
    }
    else
    {
        // The text before the comma is the command name.
        cmd.name = content.substr(0, comma);

        // The text after the comma is the command value.
        float value;

        if (!parseCommandValue(content.substr(comma + 1), value))
            return;

        cmd.hasValue = true;
        cmd.value = value;
    }

    if (cmd.name.empty())
        return;

    for (const auto& entry : handlers)
    {
        if (entry.name == cmd.name)
            entry.handler(cmd);
    }
}
