#pragma once
#include <asio/buffer.hpp>
#include <ranges>
#include <cstddef>
#include <bit>
#include <variant>
#include <system_error>
namespace bytes {
template <typename T> using expected = std::variant<T, std::error_code>;
inline auto to_range(asio::const_buffer buffer) noexcept
{
    auto byte_ptr = static_cast<std::byte const *>(buffer.data());
    return std::ranges::subrange(byte_ptr, byte_ptr + buffer.size());
}
inline auto to_range(asio::mutable_buffer buffer) noexcept
{
    auto byte_ptr = static_cast<std::byte *>(buffer.data());
    return std::ranges::subrange(byte_ptr, byte_ptr + buffer.size());
}
template <typename T, std::ranges::range State> struct state_result {
    expected<T> result;
    State state;
};
template <std::integral T, std::endian Endian = std::endian::native>
inline constexpr auto parse(std::ranges::range auto &&range)
    -> state_result<T, decltype(range)>
{
    auto count = sizeof(T);
    auto value = T{};
    if constexpr (Endian == std::endian::little) {
        while (count--) {
        }
    } else {
    }
}
} // namespace bytes
