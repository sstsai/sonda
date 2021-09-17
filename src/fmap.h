#pragma once
#include <concepts>
#include <type_traits>
#include <optional>
#include <variant>
#include <memory>
namespace functional {
// clang-format off
template <template <class...> class F, class T, class... Args>
concept Functor = requires(F<T, Args...> a, ) {
    requires std::constructible_from<F<T, Args...>, T>;
    {map(a, std::regular_invocable<)} -> std::convertible_to<T>;
};

template <template <class...> class M, class T, class... Args>
concept Monad = requires(M<T, Args...> a) {
    requires std::constructible_from<M<T, Args...>, T>;
    {unwrap(a)} -> std::convertible_to<T>;
};
// clang-format on

inline constexpr struct unwrap_fn {
    template <typename T, typename Allocator>
    friend constexpr auto tag_invoke(unwrap_fn, std::invocable<T> auto &&fn,
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
    friend constexpr auto tag_invoke(unwrap_fn, std::invocable<T> auto &&fn,
                                     std::optional<T> const &a)
        -> std::optional<std::invoke_result_t<decltype(fn), T>>
    {
        if (a)
            return std::forward<decltype(fn)>(fn)(*a);
        else
            return {};
    }
    template <typename T, typename E>
    friend constexpr auto tag_invoke(unwrap_fn, std::invocable<T> auto &&fn,
                                     std::variant<T, E> const &a)
        -> std::variant<std::invoke_result_t<decltype(fn), T>, E>
    {
        if (auto *val = std::get_if<0>(&a))
            return std::forward<decltype(fn)>(fn)(*val);
        else
            return std::get<1>(a);
    }
    template <typename Fn>
    friend constexpr auto
    tag_invoke(unwrap_fn, std::invocable<std::invoke_result_t<Fn>> auto &&fn,
               Fn &&gn)
        -> std::invoke_result_t<decltype(fn), std::invoke_result_t<Fn>>
    {
        return std::forward<decltype(fn)>(fn)(std::forward<Fn>(gn)());
    }
    template <typename... Ts> constexpr auto operator()(Ts &&...ts) const
    {
        return tag_invoke(*this, std::forward<Ts>(ts)...);
    }
} unwrap{};

} // namespace functional

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
    template <typename T, typename E>
    friend constexpr auto tag_invoke(fmap_fn, std::invocable<T> auto &&fn,
                                     std::variant<T, E> const &a)
        -> std::variant<std::invoke_result_t<decltype(fn), T>, E>
    {
        if (auto *val = std::get_if<0>(&a))
            return std::forward<decltype(fn)>(fn)(*val);
        else
            return std::get<1>(a);
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
