#pragma once
#include "bytes.h"
#include "endian.h"
#include <asio/ip/tcp.hpp>
#include <asio/read_until.hpp>
#include <asio/awaitable.hpp>
#include <asio/use_awaitable.hpp>
#include <asio/experimental/as_tuple.hpp>
#include <vector>
#include <string>
#include <optional>
#include <numeric>
#include <system_error>
namespace mqtt {
enum class error : uint8_t {
    malformed_packet = 0x81,
    protocol_error = 0x82,
    receive_maximum_exceeded = 0x93,
    packet_too_large = 0x95,
    retain_not_supported = 0x9a,
    qos_not_supported = 0x9b,
    shared_subscriptions_not_supported = 0x9e,
    subscription_identifiers_not_supported = 0xa1,
    wildcard_subscriptions_not_supported = 0xa2,
};
namespace detail {
struct error_category : std::error_category {
    const char *name() const noexcept override { return "mqtt"; }
    std::string message(int ev) const override
    {
        switch (static_cast<error>(ev)) {
        case error::malformed_packet:
            return "malformed packet";
        case error::protocol_error:
            return "protocol error";
        case error::receive_maximum_exceeded:
            return "receive maximum exceeded";
        case error::packet_too_large:
            return "packet too large";
        case error::retain_not_supported:
            return "retain not supported";
        case error::qos_not_supported:
            return "qos not supported";
        case error::shared_subscriptions_not_supported:
            return "shared subscriptions not supported";
        case error::subscription_identifiers_not_supported:
            return "subscription identifiers not supported";
        case error::wildcard_subscriptions_not_supported:
            return "wildcard subscriptions not supported";
        default:
            return "unknown error";
        }
    }
};
inline error_category &category()
{
    static error_category c;
    return c;
}
inline std::error_code make_error_code(error e)
{
    return {static_cast<int>(e), category()};
}
} // namespace detail
} // namespace mqtt
namespace std {
template <> struct is_error_code_enum<mqtt::error> : true_type {};
} // namespace std
namespace mqtt::packet {
inline auto has_extension_bit(std::byte b) -> bool
{
    return std::to_integer<bool>(b & std::byte{0x80});
}
inline auto parse_variable_byte(std::byte b) -> std::pair<bool, uint8_t>
{
    return {std::to_integer<bool>(b & std::byte{0x80}),
            std::to_integer<uint8_t>(b & std::byte{0x7f})};
}
template <typename Iterator>
inline auto parse_variable_integer(Iterator begin, Iterator end)
    -> std::pair<std::optional<uint32_t>, Iterator>
{
    uint32_t value{0};
    auto count = 0;
    for (auto iter = begin; iter < end;) {
        auto [has_continuation_bit, data] =
            parse_variable_byte(std::byte{*iter++});
        value = (value << 7) | data;
        if (!has_continuation_bit) {
            return {value, iter};
        }
        // error if continuation bit on 4th byte
        if (++count >= 4)
            break;
    }
    return {{}, begin};
}
template <typename T, typename Iterator>
inline auto parse_byte_integer(Iterator begin, Iterator end)
    -> std::pair<std::optional<T>, Iterator>
{
    constexpr auto size = sizeof(T);
    if (std::distance(begin, end) < size)
        return {{}, begin};
    end = begin + size;
    return {std::accumulate(begin, end, T{0},
                            [](T sum, auto byte) {
                                return static_cast<T>((sum << 8) | byte);
                            }),
            end};
}
template <typename Iterator>
inline auto parse_string(Iterator begin, Iterator end)
    -> std::pair<std::optional<std::string>, Iterator>
{
    auto [length, iter] = parse_byte_integer<uint16_t>(begin, end);
    if (!length || std::distance(iter, end) < *length)
        return {{}, begin};
    end = iter + *length;
    return {std::string(iter, end), end};
}
struct match_message {
    template <typename Iterator>
    auto operator()(Iterator begin, Iterator end) const
        -> std::pair<Iterator, bool>
    {
        if (begin == end)
            return std::pair{begin, false};
        auto iter = begin;
        ++iter; // control byte
        auto [value, rv] = parse_variable_integer(iter, end);
        iter             = rv;
        if (value && std::distance(iter, end) >= *value) {
            std::advance(iter, *value);
            return {iter, true};
        }
        return {begin, false};
    }
};
enum class type : uint8_t {
    connect = 1 << 4,
    connack = 2 << 4,
    publish = 3 << 4,
    puback = 4 << 4,
    pubrec = 5 << 4,
    pubrel = 6 << 4,
    pubcomp = 7 << 4,
    subscribe = 8 << 4,
    suback = 9 << 4,
    unsubscribe = 10 << 4,
    unsuback = 11 << 4,
    pingreq = 12 << 4,
    pingresp = 13 << 4,
    disconnect = 14 << 4,
    auth = 15 << 4,
};
} // namespace mqtt::packet
namespace asio {
template <>
struct is_match_condition<mqtt::packet::match_message> : public std::true_type {
};
} // namespace asio
namespace mqtt {
struct session {
    asio::ip::tcp::socket socket;
    std::vector<std::string> subscriptions;
};
template <typename Stream> inline asio::awaitable<void> connect(Stream &s)
{
    constexpr auto awaitable =
        asio::experimental::as_tuple(asio::use_awaitable);
    std::vector<std::byte> buffer;
    auto [ec, n] = co_await asio::async_read_until(
        s, asio::dynamic_buffer(buffer), packet::match_message(), awaitable);
}
} // namespace mqtt
