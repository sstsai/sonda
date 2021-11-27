#pragma once
#include <boost/mp11.hpp>
#include <compare>
#include <ratio>
#include <string_view>
#include <algorithm>
namespace strong {
namespace operations {
struct addable {};
struct subtractable {};
struct multipliable {};
struct dividable {};
} // namespace operations
// clang-format off
template <typename T>
concept Type = requires(T a)
{
    typename T::rep;
    T(typename T::rep{});
    a.value;
    {a.count()} -> std::same_as<typename T::rep>;
};
template <typename T>
concept Scalar = Type<T> && requires
{
    typename T::period;
    {T::period::num} -> std::same_as<std::intmax_t>;
    {T::period::den} -> std::same_as<std::intmax_t>;
};
template <typename T>
concept Addable = Type<T> &&
  boost::mp11::mp_contains<typename T::operators, operations::addable>::value &&
  requires(T a, T b)
{
    {a.count() + b.count()} -> std::convertible_to<typename T::rep>;
};
template <typename T>
concept Subtractable = Type<T> &&
  boost::mp11::mp_contains<typename T::operators, operations::subtractable>::value &&
  requires(T a, T b)
{
    {a.count() - b.count()} -> std::convertible_to<typename T::rep>;
};
template <typename T>
concept Multipliable = Type<T> &&
  boost::mp11::mp_contains<typename T::operators, operations::multipliable>::value &&
  requires(T a, T b)
{
    {a.count() * b.count()} -> std::convertible_to<typename T::rep>;
};
template <typename T>
concept Dividable = Type<T> &&
  boost::mp11::mp_contains<typename T::operators, operations::dividable>::value &&
  requires(T a, T b)
{
    {a.count() / b.count()} -> std::convertible_to<typename T::rep>;
};
// clang-format on
template <class T, class Unit, class Scalar = std::ratio<1>,
          class... Operations>
struct base {
    using rep       = T;
    using period    = Scalar;
    using operators = boost::mp11::mp_list<Operations...>;
    rep value;
    constexpr base() : value{} {}
    explicit constexpr base(T v) : value(v) {}
    constexpr auto count() const -> rep { return value; }
};
template <Addable T> constexpr auto operator+(T const &a, T const &b) -> T
{
    return T(a.count() + b.count());
}
template <Addable T> constexpr auto operator+=(T &a, T const &b) -> T &
{
    a.value += b.count();
    return a;
}
template <Subtractable T> constexpr auto operator-(T const &a, T const &b) -> T
{
    return T(a.count() - b.count());
}
template <Subtractable T> constexpr auto operator-=(T &a, T const &b) -> T &
{
    a.value -= b.count();
    return a;
}
template <Multipliable T> constexpr auto operator*(T const &a, T const &b) -> T
{
    return T(a.count() * b.count());
}
template <Multipliable T> constexpr auto operator*=(T &a, T const &b) -> T &
{
    a.value *= b.count();
    return a;
}
template <Dividable T> constexpr auto operator/(T const &a, T const &b) -> T
{
    return T(a.count() / b.count());
}
template <Dividable T> constexpr auto operator/=(T &a, T const &b) -> T &
{
    a.value /= b.count();
    return a;
}
} // namespace strong
namespace si {
namespace name {
template <size_t N> struct StringLiteral {
    char value[N];
    constexpr StringLiteral(const char (&str)[N])
    {
        std::copy_n(str, N, value);
    }
    constexpr operator std::string_view() const { return {value}; }
};
template <StringLiteral Name, StringLiteral Symbol> struct unit {
    static constexpr auto name   = Name;
    static constexpr auto symbol = Symbol;
};
using second = unit<"second", "s">;
using minute = unit<"minute", "min">;
} // namespace name
namespace derived {
template <class Derived, class Base = Derived, auto BaseUnits = 1> struct unit {
    using base                   = Base;
    static constexpr auto name   = Derived::name;
    static constexpr auto symbol = Derived::symbol;
};
using second = unit<name::second>;
using minute = unit<name::minute, name::second, 60>;
} // namespace derived
namespace metric {
}
template <class T, class Unit> struct base {
    using units = Unit;
    T value;
    constexpr base() : value{} {}
    explicit constexpr base(T v) : value(v) {}
    auto operator<=>(base<T, Unit> const &) const = default;
};
} // namespace si
