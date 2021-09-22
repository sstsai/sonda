#pragma once
#include <boost/describe.hpp>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <bit>
#include <concepts>
namespace bytes {
template <typename T>
concept Described = requires
{
    typename boost::describe::describe_members<T, boost::describe::mod_public>;
};
inline constexpr struct to_fn {
    friend constexpr auto tag_invoke(to_fn, std::byte *ptr,
                                     std::integral auto value) -> std::byte *
    {
        if constexpr (sizeof(value) == 1) {
            *ptr++ = std::byte{static_cast<unsigned char>(value)};
            return ptr;
        } else {
            constexpr auto size = sizeof(decltype(value));
            constexpr auto bits = std::numeric_limits<unsigned char>::digits;
            for (auto i = 0; i < size; ++i) {
                *ptr++ = std::byte{
                    static_cast<unsigned char>(value >> bits * i & 0xff)};
            }
            return ptr;
        }
    }
    template <typename T>
    requires std::is_enum_v<T>
    friend constexpr auto tag_invoke(to_fn, std::byte *ptr, T value)
        -> std::byte *
    {
        return tag_invoke(to_fn{}, ptr,
                          static_cast<std::underlying_type_t<T>>(value));
    }
    template <typename T>
    requires std::is_same_v<T, float> || std::is_same_v<T, double>
    friend constexpr auto tag_invoke(to_fn, std::byte *ptr, T value)
        -> std::byte *
    {
        if constexpr (sizeof(T) == 4) {
            return tag_invoke(to_fn{}, ptr, std::bit_cast<uint32_t>(value));
        } else if constexpr (sizeof(T) == 8) {
            return tag_invoke(to_fn{}, ptr, std::bit_cast<uint64_t>(value));
        }
    }
    template <Described T>
    friend constexpr auto tag_invoke(bytes::to_fn, std::byte *ptr, T const &obj)
        -> std::byte *
    {
        using namespace boost::describe;
        using namespace boost::mp11;
        using bases = describe_bases<T, mod_public>;
        mp_for_each<bases>([&](auto D) {
            using B = typename decltype(D)::type;
            ptr     = tag_invoke(to_fn{}, ptr, static_cast<B const &>(obj));
        });
        using members = describe_members<T, mod_public>;
        mp_for_each<members>(
            [&](auto D) { ptr = tag_invoke(to_fn{}, ptr, obj.*D.pointer); });
        return ptr;
    }
    template <typename State, typename Arg, typename... Args>
    constexpr auto operator()(State &&state, Arg &&arg, Args &&...args) const
    {
        auto rv = tag_invoke(*this, std::forward<State>(state),
                             std::forward<Arg>(arg));
        if constexpr (sizeof...(Args) > 0)
            return (*this)(rv, std::forward<Args>(args)...);
        else
            return rv;
    }
} to;
template <typename T> struct from_result {
    std::byte const *state;
    T                result;
};
inline constexpr struct from_fn {
    template <std::integral T>
    friend constexpr auto tag_invoke(from_fn, std::byte const *ptr, T &value)
        -> std::byte const *
    {
        if constexpr (sizeof(T) == 1) {
            value = static_cast<T>(*ptr++);
            return ptr;
        } else {
            constexpr auto size = sizeof(T);
            constexpr auto bits = std::numeric_limits<unsigned char>::digits;
            value               = T{};
            for (auto i = 0; i < size; ++i) {
                value |= (std::to_integer<T>(*ptr++) << bits * i);
            }
            return ptr;
        }
    }
    template <typename T>
    requires std::is_enum_v<T>
    friend constexpr auto tag_invoke(from_fn, std::byte const *ptr, T &value)
        -> std::byte const *
    {
        using underlying = typename std::underlying_type_t<T>;
        underlying ivalue;
        ptr = tag_invoke(from_fn{}, ptr, ivalue);
        value       = T(ivalue);
        return ptr;
    }
    template <typename T>
    requires std::is_same_v<T, float> || std::is_same_v<T, double>
    friend constexpr auto tag_invoke(from_fn, std::byte const *ptr, T &value)
        -> std::byte const *
    {
        if constexpr (sizeof(T) == 4) {
            uint32_t ivalue;
            ptr   = tag_invoke(from_fn{}, ptr, ivalue);
            value = std::bit_cast<T>(ivalue);
            return ptr;
        } else if constexpr (sizeof(T) == 8) {
            uint64_t ivalue;
            ptr   = tag_invoke(from_fn{}, ptr, ivalue);
            value = std::bit_cast<T>(ivalue);
            return ptr;
        }
    }
    template <Described T>
    friend constexpr auto tag_invoke(bytes::from_fn, std::byte const *ptr,
                                     T &obj) -> std::byte const *
    {
        using namespace boost::describe;
        using namespace boost::mp11;
        using bases = describe_bases<T, mod_public>;
        mp_for_each<bases>([&](auto D) {
            using B = typename decltype(D)::type;
            ptr     = tag_invoke(from_fn{}, ptr, static_cast<B &>(obj));
        });
        using members = describe_members<T, mod_public>;
        mp_for_each<members>([&](auto D) {
            using M = decltype(obj.*D.pointer);
            ptr     = tag_invoke(from_fn{}, ptr, obj.*D.pointer);
        });
        return ptr;
    }
    template <typename State, typename Arg, typename... Args>
    constexpr auto operator()(State &&state, Arg &&arg, Args &&...args) const
    {
        auto rv = tag_invoke(*this, std::forward<State>(state),
                             std::forward<Arg>(arg));
        if constexpr (sizeof...(Args) > 0)
            return (*this)(rv, std::forward<Args>(args)...);
        else
            return rv;
    }
} from;
} // namespace bytes
