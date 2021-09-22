#pragma once
#include "bytes.h"
#include <cstdint>
#include <type_traits>
namespace tiff {
inline constexpr auto header_size = 8;
inline constexpr auto ifd_size    = 12;
enum class types : uint16_t {
    byte,
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
enum class compressions : uint16_t {
    none     = 1,
    ccitt_3  = 2,
    packbits = 32773,
};
} // namespace tiff
