#pragma once
#include "mtlutils.hpp"

namespace mutils {

template <typename accum, typename remove>
constexpr auto without_names2(accum a, remove, typeset<>)
{
  return a;
}

template <typename accum, typename remove, typename name1, typename... names>
constexpr auto without_names2(accum, remove, typeset<name1, names...>, std::enable_if_t<!remove::template contains<typename name1::name>()>* = nullptr);

template <typename accum, typename remove, typename name1, typename... names>
constexpr auto without_names2(accum, remove, typeset<name1, names...>, std::enable_if_t<remove::template contains<typename name1::name>()>* = nullptr)
{
  return without_names2(accum{}, remove{}, typeset<names...>{});
}

template <typename accum, typename remove, typename name1, typename... names>
constexpr auto without_names2(accum, remove, typeset<name1, names...>, std::enable_if_t<!remove::template contains<typename name1::name>()>*)
{
  return without_names2(accum::template add<name1>(), remove{}, typeset<names...>{});
}

template <typename remove, typename... vars>
constexpr auto without_names(remove r, typeset<vars...> a)
{
  return without_names2(typeset<>{}, r, a);
}
}
