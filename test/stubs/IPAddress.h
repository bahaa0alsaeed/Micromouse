/**
 * @file IPAddress.h
 *
 * @brief Minimal IPv4 address class for native Telemetry tests.
 *
 * Implements only the operations the Telemetry library and its tests
 * use: default construction (0.0.0.0), construction from four octets,
 * strict dotted-quad parsing and equality comparison.
 */

#ifndef IPAddress_h
#define IPAddress_h

#include <cstdint>

/**
 * @brief Minimal IPv4 address representation.
 */
class IPAddress
{
public:
    /// Constructs 0.0.0.0.
    IPAddress() = default;

    /**
     * @brief Constructs an address from four octets.
     *
     * @param first First octet (most significant).
     * @param second Second octet.
     * @param third Third octet.
     * @param fourth Fourth octet (least significant).
     */
    IPAddress(uint8_t first, uint8_t second, uint8_t third, uint8_t fourth)
        : m_octets{first, second, third, fourth}
    {
    }

    /**
     * @brief Parses a dotted-quad IPv4 address.
     *
     * Accepts exactly four decimal octets in the range 0-255 separated
     * by single dots, with no leading sign, whitespace or trailing
     * characters.
     *
     * @param text Null-terminated address string.
     * @return true when the text is a valid dotted-quad address; false
     *         otherwise. On failure the address is left unchanged.
     */
    bool fromString(const char* text)
    {
        if (text == nullptr)
            return false;

        uint8_t parsed[4] = {0, 0, 0, 0};
        const char* cursor = text;

        for (int octet = 0; octet < 4; ++octet)
        {
            if (*cursor < '0' || *cursor > '9')
                return false;

            unsigned int value = 0;
            int digits = 0;

            while (*cursor >= '0' && *cursor <= '9')
            {
                value = value * 10u + static_cast<unsigned int>(*cursor - '0');

                if (++digits > 3 || value > 255)
                    return false;

                ++cursor;
            }

            parsed[octet] = static_cast<uint8_t>(value);

            if (octet < 3)
            {
                if (*cursor != '.')
                    return false;

                ++cursor;
            }
        }

        if (*cursor != '\0')
            return false;

        for (int i = 0; i < 4; ++i)
            m_octets[i] = parsed[i];

        return true;
    }

    /**
     * @brief Compares two addresses for equality.
     *
     * @param other Address to compare against.
     * @return true when all four octets are equal.
     */
    bool operator==(const IPAddress& other) const
    {
        for (int i = 0; i < 4; ++i)
        {
            if (m_octets[i] != other.m_octets[i])
                return false;
        }

        return true;
    }

    /**
     * @brief Compares two addresses for inequality.
     *
     * @param other Address to compare against.
     * @return true when at least one octet differs.
     */
    bool operator!=(const IPAddress& other) const
    {
        return !(*this == other);
    }

    /**
     * @brief Returns one octet of the address.
     *
     * @param index Octet index, 0 being the most significant.
     * @return Octet value, or 0 for an out-of-range index.
     */
    uint8_t operator[](int index) const
    {
        if (index < 0 || index > 3)
            return 0;

        return m_octets[index];
    }

private:
    /// Address octets, most significant first.
    uint8_t m_octets[4] = {0, 0, 0, 0};
};

#endif  // IPAddress_h
