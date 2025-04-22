#include <pybind11/pybind11.h>

#include <string>

#include <amulet/io/binary_reader.hpp>
#include <amulet/io/binary_writer.hpp>

namespace py = pybind11;

template <typename T>
std::string cast_to_string(const T& obj)
{
    if constexpr (std::is_arithmetic_v<T>) {
        return std::to_string(obj);
    } else if constexpr (std::is_same_v<T, std::string> || std::is_convertible_v<T, std::string>) {
        return obj;
    } else {
        return "";
    }
}

#define ASSERT_EQUAL(CLS, A, B)                        \
    {                                                  \
        CLS a;                                         \
        try {                                          \
            a = A;                                     \
        } catch (const std::exception& e) {            \
            std::string msg;                           \
            msg.reserve(200);                          \
            msg += "Failed evaluating A in file ";     \
            msg += __FILE__;                           \
            msg += " at line ";                        \
            msg += std::to_string(__LINE__);           \
            msg += ". ";                               \
            msg += e.what();                           \
            throw std::runtime_error(msg);             \
        }                                              \
        CLS b;                                         \
        try {                                          \
            b = B;                                     \
        } catch (const std::exception& e) {            \
            std::string msg;                           \
            msg.reserve(200);                          \
            msg += "Failed evaluating B in file ";     \
            msg += __FILE__;                           \
            msg += " at line ";                        \
            msg += std::to_string(__LINE__);           \
            msg += ". ";                               \
            msg += e.what();                           \
            throw std::runtime_error(msg);             \
        }                                              \
        if (a != b) {                                  \
            std::string msg;                           \
            msg.reserve(200);                          \
            msg += "Values are not equal in file ";    \
            msg += __FILE__;                           \
            msg += " at line ";                        \
            msg += std::to_string(__LINE__);           \
            msg += ".";                                \
            auto a_msg = cast_to_string(a);            \
            if (!a_msg.empty()) {                      \
                msg += " Expected \"" + a_msg + "\"."; \
            }                                          \
            auto b_msg = cast_to_string(b);            \
            if (!a_msg.empty()) {                      \
                msg += " Got \"" + b_msg + "\".";      \
            }                                          \
            throw std::runtime_error(msg);             \
        }                                              \
    }

#define ASSERT_RAISES(EXC, A)                         \
    {                                                 \
        bool err_raised = false;                      \
        try {                                         \
            A;                                        \
        } catch (const EXC&) {                        \
            err_raised = true;                        \
        } catch (...) {                               \
            std::string msg;                          \
            msg.reserve(200);                         \
            msg += "Other exception raised in file "; \
            msg += __FILE__;                          \
            msg += " at line ";                       \
            msg += std::to_string(__LINE__);          \
            msg += ". ";                              \
            throw std::runtime_error(msg);            \
        }                                             \
        if (!err_raised) {                            \
            std::string msg;                          \
            msg.reserve(200);                         \
            msg += "Exception not raised in file ";   \
            msg += __FILE__;                          \
            msg += " at line ";                       \
            msg += std::to_string(__LINE__);          \
            msg += ". ";                              \
            throw std::runtime_error(msg);            \
        }                                             \
    }

static const std::string NumericReadBufferBig("\x01\x00\x02\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x04\x3f\x80\x00\x00\x3f\xf0\x00\x00\x00\x00\x00\x00", 27);
static const std::string NumericReadBufferLittle("\x01\x02\x00\x03\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x3f\x00\x00\x00\x00\x00\x00\xf0\x3f", 27);

enum class EndianState {
    Default,
    Big,
    Little
};

void test_read_numeric(EndianState endian, bool read_offset, bool read_into)
{
    size_t index = read_offset ? 3 : 0;
    Amulet::BinaryReader reader = endian == EndianState::Big ? Amulet::BinaryReader(NumericReadBufferBig, index, std::endian::big)
        : endian == EndianState::Little                      ? Amulet::BinaryReader(NumericReadBufferLittle, index, std::endian::little)
                                                             : Amulet::BinaryReader(NumericReadBufferLittle, index);
    std::uint8_t int8 = 0;
    std::uint16_t int16 = 0;
    std::uint32_t int32 = 0;
    std::uint64_t int64 = 0;
    float float32 = 0;
    double float64 = 0;
    if (read_into) {
        if (!read_offset) {
            ASSERT_EQUAL(size_t, 0, reader.get_position())
            ASSERT_EQUAL(bool, true, reader.has_more_data())
            reader.read_numeric_into<std::uint8_t>(int8);
            ASSERT_EQUAL(size_t, 1, reader.get_position())
            ASSERT_EQUAL(bool, true, reader.has_more_data())
            reader.read_numeric_into<std::uint16_t>(int16);
        }
        ASSERT_EQUAL(size_t, 3, reader.get_position())
        ASSERT_EQUAL(bool, true, reader.has_more_data())
        reader.read_numeric_into<std::uint32_t>(int32);
        ASSERT_EQUAL(size_t, 7, reader.get_position())
        ASSERT_EQUAL(bool, true, reader.has_more_data())
        reader.read_numeric_into<std::uint64_t>(int64);
        ASSERT_EQUAL(size_t, 15, reader.get_position())
        ASSERT_EQUAL(bool, true, reader.has_more_data())
        reader.read_numeric_into<float>(float32);
        ASSERT_EQUAL(size_t, 19, reader.get_position())
        ASSERT_EQUAL(bool, true, reader.has_more_data())
        reader.read_numeric_into<double>(float64);
        ASSERT_EQUAL(size_t, 27, reader.get_position())
        ASSERT_EQUAL(bool, false, reader.has_more_data())
        if (!read_offset) {
            ASSERT_EQUAL(std::uint8_t, 1, int8)
            ASSERT_EQUAL(std::uint16_t, 2, int16)
        } else {
            ASSERT_EQUAL(std::uint8_t, 0, int8)
            ASSERT_EQUAL(std::uint16_t, 0, int16)
        }
        ASSERT_EQUAL(std::uint32_t, 3, int32)
        ASSERT_EQUAL(std::uint64_t, 4, int64)
        ASSERT_EQUAL(float, 1.0, float32)
        ASSERT_EQUAL(double, 1.0, float64)
    } else {
        if (!read_offset) {
            ASSERT_EQUAL(size_t, 0, reader.get_position())
            ASSERT_EQUAL(bool, true, reader.has_more_data())
            ASSERT_EQUAL(std::uint8_t, 1, reader.read_numeric<std::uint8_t>())
            ASSERT_EQUAL(size_t, 1, reader.get_position())
            ASSERT_EQUAL(bool, true, reader.has_more_data())
            ASSERT_EQUAL(std::uint16_t, 2, reader.read_numeric<std::uint16_t>())
        }
        ASSERT_EQUAL(size_t, 3, reader.get_position())
        ASSERT_EQUAL(bool, true, reader.has_more_data())
        ASSERT_EQUAL(std::uint32_t, 3, reader.read_numeric<std::uint32_t>())
        ASSERT_EQUAL(size_t, 7, reader.get_position())
        ASSERT_EQUAL(bool, true, reader.has_more_data())
        ASSERT_EQUAL(std::uint64_t, 4, reader.read_numeric<std::uint64_t>())
        ASSERT_EQUAL(size_t, 15, reader.get_position())
        ASSERT_EQUAL(bool, true, reader.has_more_data())
        ASSERT_EQUAL(float, 1.0, reader.read_numeric<float>())
        ASSERT_EQUAL(size_t, 19, reader.get_position())
        ASSERT_EQUAL(bool, true, reader.has_more_data())
        ASSERT_EQUAL(double, 1.0, reader.read_numeric<double>())
        ASSERT_EQUAL(size_t, 27, reader.get_position())
        ASSERT_EQUAL(bool, false, reader.has_more_data())
        ASSERT_EQUAL(std::uint8_t, 0, int8)
        ASSERT_EQUAL(std::uint16_t, 0, int16)
        ASSERT_EQUAL(std::uint32_t, 0, int32)
        ASSERT_EQUAL(std::uint64_t, 0, int64)
        ASSERT_EQUAL(float, 0.0, float32)
        ASSERT_EQUAL(double, 0.0, float64)
    }

    ASSERT_EQUAL(size_t, 27, index)
}

static const std::string StringReadBufferBig("test\x00\x0Bhello world t e s t\x00\x08 t e s t", 35);
static const std::string StringReadBufferLittle("test\x0B\x00hello world t e s t\x08\x00 t e s t", 35);

std::string odd_string_decoder(std::string_view value)
{
    std::string out;
    out.reserve(value.size() / 2);
    for (size_t i = 1; i < value.size(); i += 2) {
        out.push_back(value[i]);
    }
    return out;
}

std::string odd_string_encoder(std::string_view value)
{
    std::string out;
    out.reserve(value.size() * 2);
    for (size_t i = 0; i < value.size(); i++) {
        out.push_back(' ');
        out.push_back(value[i]);
    }
    return out;
}

void test_read_string(EndianState endian, bool read_offset)
{
    size_t index = read_offset ? 4 : 0;
    Amulet::BinaryReader reader = endian == EndianState::Big ? Amulet::BinaryReader(StringReadBufferBig, index, std::endian::big, odd_string_decoder)
        : endian == EndianState::Little                      ? Amulet::BinaryReader(StringReadBufferLittle, index, std::endian::little, odd_string_decoder)
                                                             : Amulet::BinaryReader(StringReadBufferLittle, index);
    if (!read_offset) {
        ASSERT_EQUAL(size_t, 0, reader.get_position())
        ASSERT_EQUAL(bool, true, reader.has_more_data())
        ASSERT_EQUAL(std::string, "test", reader.read_bytes(4))
    }
    ASSERT_EQUAL(size_t, 4, reader.get_position())
    ASSERT_EQUAL(bool, true, reader.has_more_data())
    ASSERT_EQUAL(std::string, "hello world", reader.read_size_and_bytes<std::uint16_t>())
    ASSERT_EQUAL(size_t, 17, reader.get_position())
    ASSERT_EQUAL(bool, true, reader.has_more_data())
    if (endian == EndianState::Default) {
        ASSERT_EQUAL(std::string, " t e s t", reader.read_string(8))
    } else {
        ASSERT_EQUAL(std::string, "test", reader.read_string(8))
    }
    ASSERT_EQUAL(size_t, 25, reader.get_position())
    ASSERT_EQUAL(bool, true, reader.has_more_data())
    if (endian != EndianState::Default) {
        ASSERT_EQUAL(std::string, "test", reader.read_size_and_string<std::uint16_t>())
        ASSERT_EQUAL(size_t, 35, reader.get_position())
        ASSERT_EQUAL(bool, false, reader.has_more_data())
    }
}

void test_read_overflow()
{
    {
        std::string value("", 0);
        size_t index = 0;
        Amulet::BinaryReader reader(value, index);
        std::uint32_t int32 = 0;
        ASSERT_RAISES(std::out_of_range, reader.read_numeric_into<std::uint32_t>(int32))
        ASSERT_RAISES(std::out_of_range, reader.read_numeric<std::uint32_t>())
        ASSERT_RAISES(std::out_of_range, reader.read_bytes(4))
        ASSERT_RAISES(std::out_of_range, reader.read_string(4))
        ASSERT_RAISES(std::out_of_range, reader.read_size_and_bytes())
        ASSERT_RAISES(std::out_of_range, reader.read_size_and_string())
        ASSERT_EQUAL(size_t, 0, reader.get_position())
        ASSERT_EQUAL(bool, false, reader.has_more_data())
    }
    {
        std::string value("\x00\x00", 2);
        size_t index = 0;
        Amulet::BinaryReader reader(value, index);
        std::uint32_t int32 = 0;
        ASSERT_RAISES(std::out_of_range, reader.read_numeric_into<std::uint32_t>(int32))
        ASSERT_RAISES(std::out_of_range, reader.read_numeric<std::uint32_t>())
        ASSERT_RAISES(std::out_of_range, reader.read_bytes(4))
        ASSERT_RAISES(std::out_of_range, reader.read_string(4))
        ASSERT_RAISES(std::out_of_range, reader.read_size_and_bytes())
        ASSERT_RAISES(std::out_of_range, reader.read_size_and_string())
        ASSERT_EQUAL(size_t, 0, reader.get_position())
        ASSERT_EQUAL(bool, true, reader.has_more_data())
    }
    {
        std::string value("\x01\x00\x00\x00", 4);
        size_t index = 0;
        Amulet::BinaryReader reader(value, index);
        std::uint32_t int32 = 0;
        ASSERT_RAISES(std::out_of_range, reader.read_size_and_bytes<std::uint32_t>())
        ASSERT_EQUAL(size_t, 4, reader.get_position())
        ASSERT_EQUAL(bool, false, reader.has_more_data())
    }
    {
        std::string value("\x01\x00\x00\x00", 4);
        size_t index = 0;
        Amulet::BinaryReader reader(value, index);
        std::uint32_t int32 = 0;
        ASSERT_RAISES(std::out_of_range, reader.read_size_and_string<std::uint32_t>())
        ASSERT_EQUAL(size_t, 4, reader.get_position())
        ASSERT_EQUAL(bool, false, reader.has_more_data())
    }
    {
        std::string value("\x02\x00\x00\x00\x00", 5);
        size_t index = 0;
        Amulet::BinaryReader reader(value, index);
        std::uint32_t int32 = 0;
        ASSERT_RAISES(std::out_of_range, reader.read_size_and_bytes<std::uint32_t>())
        ASSERT_EQUAL(size_t, 4, reader.get_position())
        ASSERT_EQUAL(bool, true, reader.has_more_data())
    }
    {
        std::string value("\x02\x00\x00\x00\x00", 5);
        size_t index = 0;
        Amulet::BinaryReader reader(value, index);
        std::uint32_t int32 = 0;
        ASSERT_RAISES(std::out_of_range, reader.read_size_and_string<std::uint32_t>())
        ASSERT_EQUAL(size_t, 4, reader.get_position())
        ASSERT_EQUAL(bool, true, reader.has_more_data())
    }
}

void test_write_numeric(EndianState endian)
{
    Amulet::BinaryWriter writer = endian == EndianState::Big ? Amulet::BinaryWriter(std::endian::big)
        : endian == EndianState::Little                      ? Amulet::BinaryWriter(std::endian::little)
                                                             : Amulet::BinaryWriter();
    writer.write_numeric<std::uint8_t>(1);
    writer.write_numeric<std::uint16_t>(2);
    writer.write_numeric<std::uint32_t>(3);
    writer.write_numeric<std::uint64_t>(4);
    writer.write_numeric<float>(1.0);
    writer.write_numeric<double>(1.0);
    if (endian == EndianState::Big) {
        ASSERT_EQUAL(std::string, NumericReadBufferBig, writer.get_buffer())
    } else {
        ASSERT_EQUAL(std::string, NumericReadBufferLittle, writer.get_buffer())
    }
}

void test_write_string(EndianState endian)
{
    Amulet::BinaryWriter writer = endian == EndianState::Big ? Amulet::BinaryWriter(std::endian::big, odd_string_encoder)
        : endian == EndianState::Little                      ? Amulet::BinaryWriter(std::endian::little, odd_string_encoder)
                                                             : Amulet::BinaryWriter();
    writer.write_bytes("test");
    writer.write_size_and_bytes<std::uint16_t>("hello world");
    if (endian == EndianState::Default) {
        ASSERT_EQUAL(std::string, StringReadBufferLittle.substr(0, 17), writer.get_buffer())
    } else {
        writer.write_string("test");
        writer.write_size_and_string<std::uint16_t>("test");
        if (endian == EndianState::Big) {
            ASSERT_EQUAL(std::string, StringReadBufferBig, writer.get_buffer())
        } else {
            ASSERT_EQUAL(std::string, StringReadBufferLittle, writer.get_buffer())
        }
    }
}

PYBIND11_MODULE(_test_amulet_io, m)
{
    py::enum_<EndianState>(m, "EndianState")
        .value("Default", EndianState::Default)
        .value("Big", EndianState::Big)
        .value("Little", EndianState::Little);

    m.def("test_read_numeric", &test_read_numeric);
    m.def("test_read_string", &test_read_string);
    m.def("test_read_overflow", &test_read_overflow);
    m.def("test_write_numeric", &test_write_numeric);
    m.def("test_write_string", &test_write_string);
}
