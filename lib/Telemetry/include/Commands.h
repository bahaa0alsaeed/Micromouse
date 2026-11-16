#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include "WiFi.h"
#include "WiFiUdp.h"

/**
 * Represents one received command.
 *
 * A command consists of a name and up to MAX_VALUES comma-separated
 * numeric values.
 *
 * Examples:
 *
 *     - stop                     (valueCount == 0)
 *
 *     - set_speed,200            (valueCount == 1)
 *
 *     - set_pid,1.2,0.05,0.8     (valueCount == 3)
 */
struct Command
{
    // Maximum number of numeric values carried by one command.
    static constexpr uint8_t MAX_VALUES = 8;

    std::string name;

    // Parsed numeric values. Only the first valueCount entries are
    // meaningful; the remaining entries stay zero-initialized. A
    // fixed-size array is used so no dynamic allocation is involved.
    std::array<float, MAX_VALUES> values{};

    // Number of values successfully parsed for this command. A command
    // without numeric values has a valueCount of zero.
    uint8_t valueCount = 0;
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
 *
 * Incoming frames of the form "name,value1,value2,..." are parsed in full
 * before dispatching: only commands whose every field parsed successfully
 * reach a registered handler, and malformed frames are discarded instead
 * of aborting (the original implementation threw an uncaught
 * std::invalid_argument exception for such frames).
 *
 * Instantiation and socket setup are internal; applications interact
 * with this service through Telemetry::commands.
 */
class Commands
{
public:
    /**
     * Registers a command handler using a std::function callback.
     */
    void on(const std::string& name, CommandHandler handler);

    /**
     * Registers a command handler using a regular function pointer.
     *
     * This overload allows simple standalone functions to be registered
     * without requiring the caller to explicitly construct a
     * CommandHandler object.
     */
    void on(const std::string& name, void (*handler)(const Command&));



    // Explicitly non-copyable: the services own UDP sockets and state.
    Commands(const Commands&) = delete;
    Commands& operator=(const Commands&) = delete;

private:
    friend class Telemetry;

    Commands() = default;

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

    // Called once by Telemetry::begin().
    void begin(uint16_t port);

    /**
     * Parses a received frame and dispatches the resulting command
     * to its registered handler.
     */
    void parseAndDispatch(const std::string& frame) const;

    /**
     * Checks for a received UDP packet and processes it when available.
     *
     * This function is intended to be called repeatedly from the main loop.
     */
    void update();
};
