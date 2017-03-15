#pragma once

namespace mutils {
template <typename T>
struct as_value
{
  using type = T;
  static constexpr T const* const nullp{ nullptr };
};

template <typename T>
constexpr T const* const as_value<T>::nullp;
}
