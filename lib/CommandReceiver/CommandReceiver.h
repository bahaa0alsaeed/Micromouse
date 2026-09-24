#pragma once

#include <functional>
#include <string>
#include <vector>
#include "WiFi.h"
#include "WiFiUdp.h"

extern const char* WIFI_SSID;
extern const char* WIFI_PASSWORD;

/**
 * Represents one received command.
 *
 * A command consists of a name and may optionally contain a numeric value.
 *
 * Examples:
 *
 *     - stop
 *
 *     - set_speed,200
 */
struct Command
{
    std::string name;

    // Indicates whether the received command contained a value.
    bool hasValue = false;

    // Numeric value associated with the command when one is present.
    float value = 0.0f;
};

/**
 * Callback type used to handle received commands.
 */
using CommandHandler = std::function<void(const Command&)>;

/**
 * Receives commands over UDP and dispatches them to registered handlers.
 *
 * Handlers are registered by command name. When a packet is received,
 * the command name is extracted and the corresponding registered handler
 * is invoked.
 *
 * The receiver itself does not define what a command does. That behavior
 * is supplied by the code that registers each command handler.
 */
class CommandReceiver
{
    /**
     * Associates a command name with its callback.
     */
    struct HandlerEntry
    {
        std::string name;
        CommandHandler handler;
    };

    // UDP socket used to receive commands.
    WiFiUDP udp;

    // Port on which commands are received.
    // A value of zero means the receiver has not been started.
    uint16_t listenPort = 0;

    // Registered command handlers.
    std::vector<HandlerEntry> handlers;

    // Reusable buffer containing the most recently received UDP frame.
    std::string rxBuffer;

    /**
     * Parses a received frame and dispatches the resulting command
     * to its registered handler.
     */
    void parseAndDispatch(const std::string& frame);

public:
    CommandReceiver();

    /**
     * Starts listening for commands on the specified UDP port.
     */
    void begin(uint16_t port);

    /**
     * Registers a command handler using a std::function callback.
     */
    void onCommand(
        const std::string& name,
        CommandHandler handler);

    /**
     * Registers a command handler using a regular function pointer.
     *
     * This overload allows simple standalone functions to be registered
     * without requiring the caller to explicitly construct a
     * CommandHandler object.
     */
    void onCommand(
        const std::string& name,
        void (*handler)(const Command&));

    /**
     * Checks for a received UDP packet and processes it when available.
     *
     * This function is intended to be called repeatedly from the main loop.
     */
    void update();
};

// Project-wide command receiver instance.
extern CommandReceiver commandReceiver;