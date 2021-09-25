#pragma once
#include "bytes.h"
#include <cstdint>
#include <type_traits>
namespace tiff {
inline constexpr auto header_size = 8;
inline constexpr auto ifd_size    = 12;
enum class tags : uint16_t {
    image_width                = 256,
    image_length               = 257,
    bits_per_sample            = 258,
    compression                = 259,
    photometric_interpretation = 262,
    strip_offsets              = 273,
    rows_per_strip             = 278,
    strip_byte_counts          = 279,
    x_resolution               = 282,
    y_resolution               = 283,
    resolution_unit            = 296,
};
enum class types : uint16_t {
    byte = 1,
    ascii,
    short_,
    long_,
    rational,
    sbyte,
    undefined,
    sshort,
    slong,
    srational,
    float_,
    double_,
};
enum class compressions : uint16_t {
    none     = 1,
    ccitt_3  = 2,
    packbits = 32773,
};
enum class photometric_interpretations : uint16_t {
    white_is_zero = 0,
    black_is_zero = 1,
};
enum class resolution_units : uint16_t {
    none       = 0,
    inch       = 1,
    centimeter = 2,
};
enum class byte_order : uint16_t {
    little_endian = 0x4949,
    big_endian    = 0x4d4d,
};
inline constexpr auto magic_number = uint16_t{42};
inline constexpr auto header(std::byte *out, uint32_t offset) -> std::byte *
{
    return bytes::to(out, byte_order::little_endian, magic_number, offset);
}
struct ifd_entry {
    tags     tag;
    types    type;
    uint32_t count;
    uint32_t value_or_offset;
};
BOOST_DESCRIBE_STRUCT(ifd_entry, (), (tag, type, count, value_or_offset))
template <std::same_as<ifd_entry>... Ts>
inline constexpr auto ifd(std::byte *out, Ts &&...entries) -> std::byte *
{
    constexpr auto count = static_cast<uint16_t>(sizeof...(entries));
    return bytes::to(out, count, std::forward<Ts>(entries)...);
}
inline constexpr auto grayscale_ifd(std::byte *out, uint16_t width,
                                    uint16_t height, uint16_t bits,
                                    uint32_t image_offset,
                                    uint32_t xresolution_offset,
                                    uint32_t yresolution_offset) -> std::byte *
{
    auto bytes = (bits + 7) / 8;
    return ifd(
        out, ifd_entry{tags::image_width, types::short_, 1, width},
        ifd_entry{tags::image_length, types::short_, 1, height},
        ifd_entry{tags::bits_per_sample, types::short_, 1, bits},
        ifd_entry{tags::compression, types::short_, 1,
                  static_cast<uint16_t>(compressions::none)},
        ifd_entry{
            tags::photometric_interpretation, types::short_, 1,
            static_cast<uint16_t>(photometric_interpretations::black_is_zero)},
        ifd_entry{tags::strip_offsets, types::short_, 1, image_offset},
        ifd_entry{tags::rows_per_strip, types::short_, 1, height},
        ifd_entry{tags::strip_byte_counts, types::long_, 1,
                  static_cast<uint32_t>(width * height * bytes)},
        ifd_entry{tags::x_resolution, types::rational, 1, xresolution_offset},
        ifd_entry{tags::y_resolution, types::rational, 1, yresolution_offset},
        ifd_entry{tags::resolution_unit, types::short_, 1,
                  static_cast<uint16_t>(resolution_units::none)});
}
} // namespace tiff
