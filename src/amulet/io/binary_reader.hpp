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

#include "endian.hpp"

namespace Amulet {

typedef std::function<std::string(std::string_view)> StringDecoder;

template <EncodedEndianness Endianness, bool HasStringDecoder>
class TemplateBinaryReader {
private:
    struct Empty { };
    std::string_view _buffer;
    size_t _position;
    std::conditional_t<Endianness.is_static, Empty, std::endian> _endianness;
    std::conditional_t<HasStringDecoder, StringDecoder, Empty> _string_decoder;

public:
    /*
     * Constructor
     *
     * @param buffer The input bytes to read.
     * @param position The index in the buffer to start at.
     */
    TemplateBinaryReader(
        std::string_view buffer,
        size_t position)
        requires(Endianness.is_static && !HasStringDecoder)
        : _buffer(buffer)
        , _position(position)
    {
    }

    /*
     * Constructor
     *
     * @param buffer The input bytes to read.
     * @param position The index in the buffer to start at.
     * @param endianness The endianness numerical values are stored in.
     */
    TemplateBinaryReader(
        std::string_view buffer,
        size_t position,
        std::endian endianness = std::endian::little)
        requires(!Endianness.is_static && !HasStringDecoder)
        : _buffer(buffer)
        , _position(position)
        , _endianness(endianness)
    {
    }

    /*
     * Constructor
     *
     * @param buffer The input bytes to read.
     * @param position The index in the buffer to start at.
     * @param endianness The endianness numerical values are stored in.
     * @param string_decoder A function to decode binary data.
     */
    TemplateBinaryReader(
        std::string_view buffer,
        size_t position,
        StringDecoder string_decoder = [](std::string_view value) { return std::string(value); })
        requires(Endianness.is_static && HasStringDecoder)
        : _buffer(buffer)
        , _position(position)
        , _string_decoder(std::move(string_decoder))
    {
    }

    /*
     * Constructor
     *
     * @param buffer The input bytes to read.
     * @param position The index in the buffer to start at.
     * @param endianness The endianness numerical values are stored in.
     * @param string_decoder A function to decode binary data.
     */
    TemplateBinaryReader(
        std::string_view buffer,
        size_t position,
        std::endian endianness = std::endian::little,
        StringDecoder string_decoder = [](std::string_view value) { return std::string(value); })
        requires(!Endianness.is_static && HasStringDecoder)
        : _buffer(buffer)
        , _position(position)
        , _endianness(endianness)
        , _string_decoder(std::move(string_decoder))
    {
    }

    /**
     * Read a numeric type from the buffer into the given value and fix its endianness.
     *
     * @param value The value to read into.
     */
    template <typename T, bool ValidateSize = true>
        requires std::is_arithmetic_v<T>
    void read_numeric_into(T& value)
    {
        if constexpr (ValidateSize) {
            // Ensure the buffer is long enough
            if (_buffer.size() - _position < sizeof(T)) {
                throw std::out_of_range(std::string("Cannot read ") + typeid(T).name() + " at position " + std::to_string(_position));
            }
        }

        // Create
        const char* src = &_buffer[_position];
        char* dst = reinterpret_cast<char*>(&value);

        if constexpr (sizeof(T) == 1) {
            std::memcpy(dst, src, sizeof(T));
        } else {
            // Copy
            if constexpr (Endianness.is_static) {
                if constexpr (Endianness.static_endianness == std::endian::native) {
                    std::memcpy(dst, src, sizeof(T));
                } else {
                    std::reverse_copy(src, src + sizeof(T), dst);
                }
            } else {
                if (_endianness == std::endian::native) {
                    std::memcpy(dst, src, sizeof(T));
                } else {
                    std::reverse_copy(src, src + sizeof(T), dst);
                }
            }
        }

        // Increment position
        _position += sizeof(T);
    }

    /**
     * Read a numeric type from the buffer and fix its endianness.
     *
     * @return A value of the requested type.
     */
    template <typename T, bool ValidateSize = true>
        requires std::is_arithmetic_v<T>
    T read_numeric()
    {
        T value;
        read_numeric_into<T, ValidateSize>(value);
        return value;
    }

    /**
     * Read a sequence of numeric types from the buffer into a vector-like object and fix their endianness.
     *
     * @param vec The vector to read into.
     * @param count The number of values to read.
     */
    template <typename T, bool ValidateSize = true, typename VecT>
        requires std::is_arithmetic_v<T>
    void read_numeric_array(VecT& vec, size_t count)
    {
        if constexpr (ValidateSize) {
            // Ensure the buffer is long enough
            if (_buffer.size() - _position < sizeof(T) * count) {
                throw std::out_of_range(std::string("Cannot read ") + std::to_string(count) + " * " + typeid(T).name() + " at position " + std::to_string(_position));
            }
        }

        // Reserve to avoid resizing the buffer.
        vec.reserve(vec.size() + count);

        T value;
        for (size_t i = 0; i < count; i++) {
            // Read the value without bounds checking.
            read_numeric_into<T, false>(value);
            vec.emplace_back(value);
        }
    }

    /*
     * Read the number of bytes.
     *
     * @param length The number of bytes to read.
     * @return The bytes.
     */
    template <bool ValidateSize = true>
    std::string_view read_bytes(size_t length)
    {
        if constexpr (ValidateSize) {
            // Ensure the buffer is long enough
            if (_buffer.size() - _position < length) {
                throw std::out_of_range("Cannot read string at position " + std::to_string(_position));
            }
        }

        std::string_view value = _buffer.substr(_position, length);
        _position += length;
        return value;
    }

    /*
     * Read the number of bytes and decode before returning.
     *
     * @param length The number of bytes to read.
     * @return The decoded string.
     */
    std::string read_string(size_t length)
        requires HasStringDecoder
    {
        return _string_decoder(read_bytes(length));
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
        requires HasStringDecoder
    std::string read_size_and_string()
    {
        return _string_decoder(read_size_and_bytes<SizeT>());
    }

    // Get the current read position.
    size_t get_position()
    {
        return _position;
    }

    // Is there more unread data.
    bool has_more_data()
    {
        return _position < _buffer.size();
    }
};

class BinaryReader : public TemplateBinaryReader<RuntimeEndian, true> {
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
        size_t position,
        std::endian endianness = std::endian::little,
        StringDecoder string_decoder = [](std::string_view value) { return std::string(value); })
        : TemplateBinaryReader(buffer, position, endianness, std::move(string_decoder))
    {
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
    BinaryReader reader(buffer, 0);
    return deserialise<T>(reader);
}

} // namespace Amulet
