#pragma once
namespace strong {
template <typename T>
concept Type = requires(T a)
{
    typename T::type;
    typename T::value_type;
    {
        a.value
    }
    noexcept->std::same_as<T::value_type>;
};
template <typename T>
concept Addable = Type<T> && requires
{
    typename T::addable;
};
template <Addable T> auto operator+(T const &a, T const &b) -> T
{
    return a.value + b.value;
}
} // namespace strong
