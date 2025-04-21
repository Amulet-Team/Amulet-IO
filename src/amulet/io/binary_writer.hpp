#pragma once

#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>
#include <string>
#include <limits>

namespace Amulet {

typedef std::function<std::string(const std::string&)> StringEncoder;

class BinaryWriter {
private:
    std::string data;
    std::endian endianness;
    StringEncoder string_encoder;

public:
    /* Constructor
     *
     * @param endianness The endianness numerical values are stored in.
     * @param string_encoder A function to encode binary data.
     */
    BinaryWriter(
        std::endian endianness = std::endian::little,
        StringEncoder string_encoder = [](const std::string& value) { return value; })
        : endianness(endianness)
        , string_encoder(string_encoder)
    {
    }

    // Fix the endianness of the numeric value and write it to the buffer.
    template <typename T>
    void write_numeric(const T& value)
    {
        if (endianness == std::endian::native) {
            data.append((char*)&value, sizeof(T));
        } else {
            T value_reverse;
            char* src = (char*)&value;
            char* dst = (char*)&value_reverse;
            for (size_t i = 0; i < sizeof(T); i++) {
                dst[i] = src[sizeof(T) - i - 1];
            }
            data.append(dst, sizeof(T));
        }
    }

    // Encode and return a string.
    std::string encode_string(const std::string& value)
    {
        return string_encoder(value);
    }

    // Write bytes without prefixing a size.
    void write_bytes(const std::string& value)
    {
        data.append(value);
    }

    // Write a string without prefixing a size.
    void write_string(const std::string& value)
    {
        write_bytes(string_encoder(value));
    }

    // Write the bytes to the buffer with a prefixed size.
    template <typename SizeT = std::uint64_t>
    void write_size_and_bytes(const std::string& value)
    {
        if (std::numeric_limits<SizeT>::max() < value.size()) {
            throw std::runtime_error("value is to large for the given size type.");
        }
        write_numeric<SizeT>(static_cast<SizeT>(value.size()));
        write_bytes(value);
    }

    // Encode and write a string to the buffer with a prefixed size.
    template <typename SizeT = std::uint64_t>
    void write_size_and_string(const std::string& value)
    {
        write_size_and_bytes<SizeT>(string_encoder(value));
    }

    // Get the written buffer.
    const std::string& get_buffer()
    {
        return data;
    }
};

// Utility function to searialise an object.
template <typename T>
std::string serialise(const T& obj)
{
    BinaryWriter writer;
    obj.serialise(writer);
    return writer.get_buffer();
}

} // namespace Amulet
