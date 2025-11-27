#pragma once

#include <bit>
#include <type_traits>

namespace Amulet {

struct RuntimeEndianness { };

struct EncodedEndianness {
    // If true the endianness is static and value should be used.
    bool is_static;
    // The static endianness
    std::endian static_endianness;

    constexpr EncodedEndianness(RuntimeEndianness)
        : is_static(false)
        , static_endianness(std::endian::native)
    {
    }

    constexpr EncodedEndianness(std::endian static_endianness)
        : is_static(true)
        , static_endianness(static_endianness)
    {
    }
};

constexpr EncodedEndianness StaticLittleEndian { std::endian::little };
constexpr EncodedEndianness StaticBigEndian { std::endian::big };
constexpr EncodedEndianness RuntimeEndian { RuntimeEndianness {} };

} // namespace Amulet
