#include "CommandReceiver.h"

// Project-wide command receiver instance.
CommandReceiver commandReceiver;

const char* WIFI_SSID = MY_WIFI_SSID;
const char* WIFI_PASSWORD = MY_WIFI_PASSWORD;
const char* LOGGER_HOST = MY_LOGGER_HOST;
constexpr uint16_t LOGGER_PORT = MY_LOGGER_PORT;


CommandReceiver::CommandReceiver() = default;

void CommandReceiver::begin(const uint16_t port)
{

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    {
        delay(100);
    }

    IPAddress destination;

    // Convert the configured destination address from text to an IP address.
    if (!destination.fromString(LOGGER_HOST))
    {
        Serial.println("Invalid LOGGER_HOST");
        return;
    }

    listenPort = port;
    udp.begin(port);

}

void CommandReceiver::onCommand(
    const std::string& name,
    CommandHandler handler)
{
    handlers.push_back({name, std::move(handler)});
}

void CommandReceiver::onCommand(
    const std::string& name,
    void (*handler)(const Command&))
{
    // Convert the function pointer to the common handler type so both
    // registration overloads use the same underlying implementation.
    onCommand(name, CommandHandler(handler));
}


void CommandReceiver::update()
{
    // A port of zero indicates that the receiver has not been initialized.
    if (listenPort == 0)
        return;

    // Commands can only be received while the Wi-Fi connection is active.
    if (WiFi.status() != WL_CONNECTED)
        return;

    const int packetSize = udp.parsePacket();

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

void CommandReceiver::parseAndDispatch(
    const std::string& frame)
{
    std::string content = frame;

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
        const std::string value = content.substr(comma + 1);

        cmd.hasValue = true;
        cmd.value = std::stof(value);
    }

    if (cmd.name.empty())
        return;

    for (const auto& entry : handlers)
    {
        if (entry.name == cmd.name)
            entry.handler(cmd);
    }
}