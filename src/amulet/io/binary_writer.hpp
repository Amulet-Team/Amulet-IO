#pragma once

#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>
#include <string>

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

    // Write a string without encoding or prefixing a size.
    void write_bytes(const std::string& value)
    {
        data.append(value);
    }

    // Write the bytes to the buffer with a prefixed unsigned 64 bit integer size.
    void writeSizeAndBytes(const std::string& value)
    {
        writeNumeric<std::uint64_t>(value.size());
        data.append(value);
    }

    // Get the written buffer.
    const std::string& get_buffer()
    {
        return data;
    }
};

} // namespace Amulet


// Utility function to searialise an object.
template <typename T>
std::string serialise(const T& obj)
{
    BinaryWriter writer;
    obj.serialise(writer);
    return writer.getBuffer();
}
}
