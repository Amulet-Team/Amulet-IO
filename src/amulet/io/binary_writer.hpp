#pragma once

#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstring>
#include <functional>
#include <limits>
#include <string>

#include "endian.hpp"

namespace Amulet {

typedef std::function<std::string(const std::string&)> StringEncoder;

template <EncodedEndianness Endianness, bool HasStringEncoder>
class TemplateBaseBinaryWriter {
private:
    struct Empty { };
    std::string& _buffer;
    std::conditional_t<Endianness.is_static, Empty, std::endian> _endianness;
    std::conditional_t<HasStringEncoder, StringEncoder, Empty> _string_encoder;

public:
    /* Constructor
     *
     * @param buffer The string to write to.
     */
    TemplateBaseBinaryWriter(std::string& buffer)
        requires(Endianness.is_static && !HasStringEncoder)
        : _buffer(buffer)
    {
    }

    /* Constructor
     *
     * @param buffer The string to write to.
     * @param endianness The endianness numerical values are stored in.
     */
    TemplateBaseBinaryWriter(
        std::string& buffer,
        std::endian endianness = std::endian::little)
        requires(!Endianness.is_static && !HasStringEncoder)
        : _buffer(buffer)
        , _endianness(endianness)
    {
    }

    /* Constructor
     *
     * @param buffer The string to write to.
     * @param string_encoder A function to encode binary data.
     */
    TemplateBaseBinaryWriter(
        std::string& buffer,
        StringEncoder string_encoder = [](const std::string& value) { return value; })
        requires(Endianness.is_static && HasStringEncoder)
        : _buffer(buffer)
        , _string_encoder(std::move(string_encoder))
    {
    }

    /* Constructor
     *
     * @param buffer The string to write to.
     * @param endianness The endianness numerical values are stored in.
     * @param string_encoder A function to encode binary data.
     */
    TemplateBaseBinaryWriter(
        std::string& buffer,
        std::endian endianness = std::endian::little,
        StringEncoder string_encoder = [](const std::string& value) { return value; })
        requires(!Endianness.is_static && HasStringEncoder)
        : _buffer(buffer)
        , _endianness(endianness)
        , _string_encoder(std::move(string_encoder))
    {
    }

    // Fix the endianness of the numeric value and write it to the buffer.
    template <typename T>
        requires std::is_arithmetic_v<T>
    void write_numeric(T value)
    {
        char* src = reinterpret_cast<char*>(&value);
        if constexpr (1 < sizeof(T)) {
            if constexpr (Endianness.is_static) {
                if constexpr (Endianness.static_endianness != std::endian::native) {
                    std::reverse(src, src + sizeof(T));
                }
            } else {
                if (_endianness != std::endian::native) {
                    std::reverse(src, src + sizeof(T));
                }
            }
        }
        _buffer.append(src, sizeof(T));
    }

    // Encode and return a string.
    std::string encode_string(const std::string& value)
    {
        return _string_encoder(value);
    }

    // Write bytes without prefixing a size.
    void write_bytes(const std::string& value)
    {
        _buffer.append(value);
    }

    // Write a string without prefixing a size.
    void write_string(const std::string& value)
        requires HasStringEncoder
    {
        write_bytes(_string_encoder(value));
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
        requires HasStringEncoder
    void write_size_and_string(const std::string& value)
    {
        write_size_and_bytes<SizeT>(_string_encoder(value));
    }

    // Get the written buffer.
    const std::string& get_buffer()
    {
        return _buffer;
    }
};

using BaseBinaryWriter = TemplateBaseBinaryWriter<RuntimeEndian, true>;

template <EncodedEndianness Endianness, bool HasStringEncoder>
class TemplateBinaryWriter : public TemplateBaseBinaryWriter<Endianness, HasStringEncoder> {
private:
    std::string _str;

public:
    template <typename... Args>
    TemplateBinaryWriter(Args&&... args)
        : TemplateBaseBinaryWriter<Endianness, HasStringEncoder>::TemplateBaseBinaryWriter(_str, std::forward<Args>(args)...)
    {
    }
};

class BinaryWriter : public TemplateBinaryWriter<RuntimeEndian, true> {
public:
    /* Constructor
     *
     * @param endianness The endianness numerical values are stored in.
     * @param string_encoder A function to encode binary data.
     */
    BinaryWriter(
        std::endian endianness = std::endian::little,
        StringEncoder string_encoder = [](const std::string& value) { return value; })
        : TemplateBinaryWriter<RuntimeEndian, true>::TemplateBinaryWriter(endianness, std::move(string_encoder))
    {
    }
};

// Utility function to searialise an object.
template <typename T>
std::string serialise(const T& obj)
{
    std::string buffer;
    BaseBinaryWriter writer(buffer);
    obj.serialise(writer);
    return buffer;
}

} // namespace Amulet
