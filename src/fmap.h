#pragma once
#include <concepts>
#include <type_traits>
#include <optional>
#include <memory>
inline constexpr struct fmap_fn {
    template <typename T, typename Allocator>
    friend constexpr auto tag_invoke(fmap_fn, std::invocable<T> auto &&fn,
                                     std::vector<T, Allocator> const &a)
        -> std::vector<std::invoke_result_t<decltype(fn), T>>
    {
        std::vector<std::invoke_result_t<decltype(fn), T>> output;
        output.reserve(a.size());
        std::transform(std::begin(a), std::end(a), std::back_inserter(output),
                       std::forward<decltype(fn)>(fn));
        return output;
    }
    template <typename T>
    friend constexpr auto tag_invoke(fmap_fn, std::invocable<T> auto &&fn,
                                     std::optional<T> const &a)
        -> std::optional<std::invoke_result_t<decltype(fn), T>>
    {
        if (a)
            return std::forward<decltype(fn)>(fn)(*a);
        else
            return {};
    }
    template <typename Fn>
    friend constexpr auto
    tag_invoke(fmap_fn, std::invocable<std::invoke_result_t<Fn>> auto &&fn,
               Fn &&gn)
        -> std::invoke_result_t<decltype(fn), std::invoke_result_t<Fn>>
    {
        return std::forward<decltype(fn)>(fn)(std::forward<Fn>(gn)());
    }
    template <typename... Ts> constexpr auto operator()(Ts &&...ts) const
    {
        return tag_invoke(*this, std::forward<Ts>(ts)...);
    }
} fmap{};
