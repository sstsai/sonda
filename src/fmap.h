#pragma once
#include <boost/mp11.hpp>
#include <concepts>
#include <type_traits>
#include <optional>
#include <variant>
#include <vector>

namespace func {
// clang-format off
template <class C, class T, class...Args>
concept range = requires(C &c)
{
    std::begin(c);
    std::end(c);
    {C(std::initializer_list<T>{})} -> std::convertible_to<C>;
    std::back_inserter(c);
};
// clang-format on
inline constexpr struct apply_fn {
    template <template <typename...> typename Sequence, typename T,
              typename... Ts, std::regular_invocable<T> Fn, typename... Us>
    requires range<Sequence<T, Ts...>, T>
    friend constexpr auto tag_invoke(apply_fn, Sequence<T, Ts...> const &a,
                                     Sequence<Fn, Us...> const &seq) noexcept
        -> Sequence<std::invoke_result_t<Fn, T>>
    {
        Sequence<std::invoke_result_t<Fn, T>> output;
        std::for_each(std::begin(seq), std::end(seq), [&](auto const &fn) {
            std::transform(std::begin(a), std::end(a),
                           std::back_inserter(output), fn);
        });
        return output;
    }
    template <typename T, std::regular_invocable<T> Fn>
    friend constexpr auto tag_invoke(apply_fn, std::optional<T> const &a,
                                     std::optional<Fn> const &fn) noexcept
        -> std::optional<std::invoke_result_t<Fn, T>>
    {
        if (a && fn)
            return (*fn)(*a);
        else
            return {};
    }
    template <template <typename...> typename Functor, typename T,
              std::regular_invocable<T> Fn, typename... Ts, typename... Us,
              typename... Args>
    constexpr auto operator()(Functor<T, Ts...> const & f,
                              Functor<Fn, Us...> const &fn,
                              Args &&...args) const noexcept
    {
        auto result = tag_invoke(*this, f, fn);
        if constexpr (sizeof...(Args) == 0)
            return result;
        else
            return (*this)(result, std::forward<Args>(args)...);
    }
} apply;

inline constexpr struct fmap_fn {
    template <template <typename...> typename Sequence, typename T,
              typename... Ts, std::regular_invocable<T> Fn>
    requires range<Sequence<T, Ts...>, T>
    friend constexpr auto tag_invoke(fmap_fn, Sequence<T, Ts...> const &a,
                                     Fn &&fn) noexcept
        -> Sequence<std::invoke_result_t<Fn, T>>
    {
        Sequence<std::invoke_result_t<Fn, T>> output;
        std::transform(std::begin(a), std::end(a), std::back_inserter(output),
                       std::forward<Fn>(fn));
        return output;
    }
    template <typename T, std::regular_invocable<T> Fn>
    friend constexpr auto tag_invoke(fmap_fn, std::optional<T> const &a,
                                     Fn &&fn) noexcept
        -> std::optional<std::invoke_result_t<Fn, T>>
    {
        if (a)
            return std::forward<Fn>(fn)(*a);
        else
            return {};
    }
    template <typename T, typename E, std::regular_invocable<T> Fn>
    friend constexpr auto tag_invoke(fmap_fn, std::variant<T, E> const &a,
                                     Fn &&fn) noexcept
        -> std::variant<std::invoke_result_t<Fn, T>, E>
    {
        if (auto *val = std::get_if<0>(&a))
            return std::forward<decltype(fn)>(fn)(*val);
        else
            return std::get<1>(a);
    }
    template <template <typename...> typename Functor, typename T,
              typename... Ts, std::regular_invocable<T> Fn, typename... Args>
    constexpr auto operator()(Functor<T, Ts...> const &f, Fn &&fn,
                              Args &&...args) const noexcept
    {
        auto result = tag_invoke(*this, f, std::forward<Fn>(fn));
        if constexpr (sizeof...(Args) == 0)
            return result;
        else
            return (*this)(result, std::forward<Args>(args)...);
    }
} fmap{};

inline constexpr struct flatten_fn {
    template <template <typename...> typename Sequence, typename T,
              typename... Ts, typename... Us>
    requires range<Sequence<T, Ts...>, T> &&
        range<Sequence<Sequence<T, Ts...>, Us...>, Sequence<T, Ts...>>
    friend constexpr auto
    tag_invoke(flatten_fn,
               Sequence<Sequence<T, Ts...>, Us...> const &a) noexcept
        -> Sequence<T, Ts...>
    {
        Sequence<T, Ts...> output;
        std::for_each(std::begin(a), std::end(a), [&output](auto const &seq) {
            std::copy(std::begin(seq), std::end(seq),
                      std::back_inserter(output));
        });
        return output;
    }
    template <typename T>
    friend constexpr auto
    tag_invoke(flatten_fn, std::optional<std::optional<T>> const &a) noexcept
        -> std::optional<T>
    {
        if (a)
            return a.value();
        else
            return {};
    }
    template <typename T, typename E>
    friend constexpr auto
    tag_invoke(flatten_fn,
               std::variant<std::variant<T, E>, E> const &a) noexcept
        -> std::variant<T, E>
    {
        if (auto *val = std::get_if<0>(&a))
            return *val;
        else
            return std::get<1>(a);
    }
    template <template <typename...> typename Functor, typename T,
              typename... Ts>
    constexpr auto operator()(Functor<T, Ts...> const &f) const noexcept
    {
        return tag_invoke(*this, f);
    }
} flatten{};

inline constexpr struct bind_fn {
    template <template <typename...> typename Functor, typename T,
              typename... Ts, std::regular_invocable<T> Fn>
    friend constexpr auto tag_invoke(bind_fn, Functor<T, Ts...> const &f,
                                     Fn &&fn) noexcept
        -> std::invoke_result_t<Fn, T>
    {
        return flatten(fmap(f, std::forward<Fn>(fn)));
    }
    template <template <typename...> typename Functor, typename T,
              typename... Ts, std::regular_invocable<T> Fn, typename... Args>
    constexpr auto operator()(Functor<T, Ts...> const &f, Fn &&fn,
                              Args &&...args) const noexcept
        -> std::invoke_result_t<Fn, T>
    {
        auto result = tag_invoke(*this, f, std::forward<Fn>(fn));
        if constexpr (sizeof...(Args) == 0)
            return result;
        else
            return (*this)(result, std::forward<Args>(args)...);
    }
} bind{};
} // namespace func
