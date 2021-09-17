#pragma once
#include <bit>
#include <cstdint>
#include <type_traits>
#ifndef BOOST_ENDIAN_NO_INTRINSICS

#ifndef __has_builtin      // Optional of course
#define __has_builtin(x) 0 // Compatibility with non-clang compilers
#endif

#if defined(_MSC_VER) && (!defined(__clang__) || defined(__c2__))
//  Microsoft documents these as being compatible since Windows 95 and
//  specifically lists runtime library support since Visual Studio 2003
//  (aka 7.1). Clang/c2 uses the Microsoft rather than GCC intrinsics, so we
//  check for defined(_MSC_VER) before defined(__clang__)
#define BOOST_ENDIAN_INTRINSIC_MSG "cstdlib _byteswap_ushort, etc."
#include <cstdlib>
#define BOOST_ENDIAN_INTRINSIC_BYTE_SWAP_2(x) _byteswap_ushort(x)
#define BOOST_ENDIAN_INTRINSIC_BYTE_SWAP_4(x) _byteswap_ulong(x)
#define BOOST_ENDIAN_INTRINSIC_BYTE_SWAP_8(x) _byteswap_uint64(x)

//  GCC and Clang recent versions provide intrinsic byte swaps via builtins
#elif (defined(__clang__) && __has_builtin(__builtin_bswap32) &&               \
       __has_builtin(__builtin_bswap64)) ||                                    \
    (defined(__GNUC__) &&                                                      \
     (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3)))
#define BOOST_ENDIAN_INTRINSIC_MSG "__builtin_bswap16, etc."
// prior to 4.8, gcc did not provide __builtin_bswap16 on some platforms so we
// emulate it see http://gcc.gnu.org/bugzilla/show_bug.cgi?id=52624 Clang has a
// similar problem, but their feature test macros make it easier to detect
#if (defined(__clang__) && __has_builtin(__builtin_bswap16)) ||                \
    (defined(__GNUC__) &&                                                      \
     (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8)))
#define BOOST_ENDIAN_INTRINSIC_BYTE_SWAP_2(x) __builtin_bswap16(x)
#else
#define BOOST_ENDIAN_INTRINSIC_BYTE_SWAP_2(x) __builtin_bswap32((x) << 16)
#endif
#define BOOST_ENDIAN_INTRINSIC_BYTE_SWAP_4(x) __builtin_bswap32(x)
#define BOOST_ENDIAN_INTRINSIC_BYTE_SWAP_8(x) __builtin_bswap64(x)

#define BOOST_ENDIAN_CONSTEXPR_INTRINSICS

//  Linux systems provide the byteswap.h header, with
#elif defined(__linux__)
//  don't check for obsolete forms defined(linux) and defined(__linux) on the
//  theory that compilers that predefine only these are so old that byteswap.h
//  probably isn't present.
#define BOOST_ENDIAN_INTRINSIC_MSG "byteswap.h bswap_16, etc."
#include <byteswap.h>
#define BOOST_ENDIAN_INTRINSIC_BYTE_SWAP_2(x) bswap_16(x)
#define BOOST_ENDIAN_INTRINSIC_BYTE_SWAP_4(x) bswap_32(x)
#define BOOST_ENDIAN_INTRINSIC_BYTE_SWAP_8(x) bswap_64(x)

#else
#define BOOST_ENDIAN_NO_INTRINSICS
#define BOOST_ENDIAN_INTRINSIC_MSG "no byte swap intrinsics"
#endif

#elif !defined(BOOST_ENDIAN_INTRINSIC_MSG)
#define BOOST_ENDIAN_INTRINSIC_MSG "no byte swap intrinsics"
#endif // BOOST_ENDIAN_NO_INTRINSICS
namespace endian {
namespace detail {
template <std::size_t N> struct integral_by_size {};
template <> struct integral_by_size<1> {
    using type = uint8_t;
};
template <> struct integral_by_size<2> {
    using type = uint16_t;
};
template <> struct integral_by_size<4> {
    using type = uint32_t;
};
template <> struct integral_by_size<8> {
    using type = uint64_t;
};
template <std::size_t N>
using integral_by_size_t = typename integral_by_size<N>::type;
inline uint8_t constexpr endian_reverse_impl(uint8_t x) noexcept { return x; }
#ifdef BOOST_ENDIAN_NO_INTRINSICS
inline constexpr uint16_t endian_reverse_impl(uint16_t x) noexcept
{
    return (x << 8) | (x >> 8);
}
inline constexpr uint32_t endian_reverse_impl(uint32_t x) noexcept
{
    uint32_t step16 = x << 16 | x >> 16;
    return ((step16 << 8) & 0xff00ff00) | ((step16 >> 8) & 0x00ff00ff);
}
inline constexpr uint64_t endian_reverse_impl(uint64_t x) noexcept
{
    uint64_t step32 = x << 32 | x >> 32;
    uint64_t step16 = (step32 & 0x0000FFFF0000FFFFULL) << 16 |
                      (step32 & 0xFFFF0000FFFF0000ULL) >> 16;
    return (step16 & 0x00FF00FF00FF00FFULL) << 8 |
           (step16 & 0xFF00FF00FF00FF00ULL) >> 8;
}
#else
inline uint16_t endian_reverse_impl(uint16_t x) noexcept
{
    return BOOST_ENDIAN_INTRINSIC_BYTE_SWAP_2(x);
}
inline uint32_t endian_reverse_impl(uint32_t x) noexcept
{
    return BOOST_ENDIAN_INTRINSIC_BYTE_SWAP_4(x);
}
inline uint64_t endian_reverse_impl(uint64_t x) noexcept
{
    return BOOST_ENDIAN_INTRINSIC_BYTE_SWAP_8(x);
}
#endif
} // namespace detail
template <typename T>
inline constexpr auto reverse(T x) noexcept
    -> std::enable_if_t<std::is_arithmetic_v<T> || std::is_enum_v<T>, T>
{
    return std::bit_cast<T>(detail::endian_reverse_impl(
        std::bit_cast<detail::integral_by_size_t<sizeof(T)>>(x)));
}
} // namespace endian
