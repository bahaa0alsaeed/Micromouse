/**
 * @file Commands.h
 *
 * @brief Receives commands over UDP and dispatches them to handlers.
 *
 * Part of the Telemetry stack; applications interact with this service
 * through Telemetry::commands. Frames of the form "name,value1,..."
 * are parsed in full before any registered handler is invoked.
 */

#pragma once

#include <array>
#include <string>
#include <vector>
#include <functional>

#include "WiFiUdp.h"

/**
 * @brief Represents one received command.
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

    // Command name, the text before the first comma.
    std::string name;

    // Parsed numeric values. Only the first valueCount entries are
    // meaningful; the remaining entries stay zero-initialized. A
    // fixed-size storage avoids dynamic allocation for numeric values.
    std::array<float, MAX_VALUES> values{};

    // Number of values successfully parsed for this command. A command
    // without numeric values has a valueCount of zero.
    uint8_t valueCount = 0;
};

/**
 * @brief Callback type used to handle received commands.
 */
using CommandHandler = std::function<void(const Command&)>;

/**
 * @brief Receives commands over UDP and dispatches them to registered handlers.
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
 * of aborting.
 *
 * Instantiation and socket setup are internal; applications interact
 * with this service through Telemetry::commands.
 */
class Commands
{
public:
    /**
     * @brief Registers a command handler using a std::function callback.
     *
     * @param name Command name to handle, matched against the text
     *             before the first comma of a received frame.
     *             Must not be empty or contain ',', '=', '\n', or '\r'.
     * @param handler Callback invoked with the parsed command.
     *                Must not be empty.
     *
     * @return true if the handler was successfully registered,
     *         false if the name or handler is invalid.
     */
    bool on(const std::string& name, CommandHandler handler);

    /**
     * @brief Registers a command handler using a regular function pointer.
     *
     * This overload allows simple standalone functions to be registered
     * without requiring the caller to explicitly construct a
     * CommandHandler object.
     *
     * @param name Command name to handle, matched against the text
     *             before the first comma of a received frame.
     *             Must not be empty or contain ',', '=', '\n', or '\r'.
     * @param handler Callback invoked with the parsed command.
     *                Must not be nullptr.
     *
     * @return true if the handler was successfully registered,
     *         false if the name or handler is invalid.
     */
    bool on(const std::string& name, void (*handler)(const Command&));

    // Explicitly non-copyable: the services own UDP sockets and state.
    Commands(const Commands&) = delete;
    Commands& operator=(const Commands&) = delete;

private:
    friend class Telemetry;

    Commands() = default;

    /**
     * @brief Associates a command name with its callback.
     */
    struct HandlerEntry
    {
        // Command name the handler responds to.
        std::string name;
        // Callback invoked when the name matches a received command.
        CommandHandler handler;
    };

    // UDP socket used to receive commands.
    WiFiUDP m_udp;

    // Port on which commands are received.
    // A value of zero means the receiver has not been started.
    uint16_t m_listenPort = 0;

    // Registered command handlers.
    std::vector<HandlerEntry> m_handlers;

    // Reusable buffer containing the most recently received UDP frame.
    std::string m_rxBuffer;

    /**
     * @brief Initializes the UDP listener on the specified port.
     *
     * @param port Local UDP port to listen on. Must not be zero.
     *
     * @return true if the UDP listener was successfully initialized,
     *         false if the port is invalid or initialization fails.
     */
    bool begin(const uint16_t port);

    /**
     * @brief Parses a received frame and dispatches the resulting command
     * to its registered handler.
     *
     * Malformed frames are silently discarded: no handler is invoked.
     * Every handler registered under the command name is invoked, in
     * registration order.
     *
     * @param frame Raw received frame.
     */
    void parseAndDispatch(const std::string& frame) const;

    /**
     * @brief Checks for a received UDP packet and processes it when available.
     *
     * Oversized frames (larger than the internal
     * MAX_COMMAND_PACKET_SIZE) are drained and discarded.
     *
     * This function is intended to be called repeatedly from the main loop
     * (Telemetry::update() does).
     */
    void update();
};
