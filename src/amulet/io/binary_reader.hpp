#pragma once

#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

namespace Amulet {

typedef std::function<std::string(std::string_view)> StringDecoder;

class BinaryReader {
private:
    std::string_view buffer;
    size_t& position;
    std::endian endianness;
    StringDecoder string_decoder;

public:
    /*
     * Constructor
     *
     * @param buffer The input bytes to read.
     * @param position The index in the buffer to start at.
     * @param endianness The endianness numerical values are stored in.
     * @param string_decoder A function to decode binary data.
     */
    BinaryReader(
        std::string_view buffer,
        size_t& position,
        std::endian endianness = std::endian::little,
        StringDecoder string_decoder = [](std::string_view value) { return std::string(value); })
        : buffer(buffer)
        , position(position)
        , endianness(endianness)
        , string_decoder(string_decoder)
    {
    }

    /**
     * Read a numeric type from the buffer into the given value and fix its endianness.
     *
     * @param value The value to read into.
     */
    template <typename T>
    void read_numeric_into(T& value)
    {
        // Ensure the buffer is long enough
        if (position + sizeof(T) > buffer.size()) {
            throw std::out_of_range(std::string("Cannot read ") + typeid(T).name() + " at position " + std::to_string(position));
        }

        // Create
        const char* src = &buffer[position];
        char* dst = (char*)&value;

        // Copy
        if (endianness == std::endian::native) {
            for (size_t i = 0; i < sizeof(T); i++) {
                dst[i] = src[i];
            }
        } else {
            for (size_t i = 0; i < sizeof(T); i++) {
                dst[i] = src[sizeof(T) - i - 1];
            }
        }

        // Increment position
        position += sizeof(T);
    }

    /**
     * Read a numeric type from the buffer and fix its endianness.
     *
     * @return A value of the requested type.
     */
    template <typename T>
    T read_numeric()
    {
        T value;
        read_numeric_into<T>(value);
        return value;
    }

    /*
     * Read the number of bytes.
     *
     * @param length The number of bytes to read.
     * @return The bytes.
     */
    std::string_view read_bytes(size_t length)
    {
        // Ensure the buffer is long enough
        if (position + length > buffer.size()) {
            throw std::out_of_range("Cannot read string at position " + std::to_string(position));
        }

        std::string_view value = buffer.substr(position, length);
        position += length;
        return value;
    }

    /*
     * Read the number of bytes and decode before returning.
     *
     * @param length The number of bytes to read.
     * @return The decoded string.
     */
    std::string read_string(size_t length)
    {
        return string_decoder(read_bytes(length));
    }

    /*
     * Read a size followed by that many bytes.
     *
     * @return A string_view to the bytes.
     */
    template <typename SizeT = std::uint64_t>
    std::string_view read_size_and_bytes()
    {
        SizeT length;
        read_numeric_into<SizeT>(length);
        return read_bytes(length);
    }

    /*
     * Read a size followed by that many bytes and return the decoded value.
     *
     * @return A string decoded from the bytes.
     */
    template <typename SizeT = std::uint64_t>
    std::string read_size_and_string()
    {
        return string_decoder(read_size_and_bytes<SizeT>());
    }

    // Get the current read position.
    size_t get_position()
    {
        return position;
    }

    // Is there more unread data.
    bool has_more_data()
    {
        return position < buffer.size();
    }
};

// Utility function to call the deserialise method on a class.
template <class T>
T deserialise(BinaryReader& reader)
{
    return T::deserialise(reader);
}

// Utility function to call the deserialise method on a class.
template <class T>
T deserialise(std::string_view buffer)
{
    size_t position = 0;
    BinaryReader reader(buffer, position);
    return deserialise<T>(reader);
}

} // namespace Amulet
