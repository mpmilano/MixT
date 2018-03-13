#pragma once

#include "mtl/parse_bindings_decl.hpp"
#include "mtl/parse_expressions_decl.hpp"

namespace myria {
namespace mtl {
template <char... str>
constexpr auto parse_binding(mutils::String<str...>)
{
  using namespace mutils;
  constexpr auto var = String<str...>::split(zero{}, String<'='>{}).trim_ends();
  constexpr auto expr = String<str...>::after_fst(String<'='>{}).trim_ends();
  return parse_phase::Binding<std::decay_t<decltype(var)>, std::decay_t<decltype(parse_expression(expr))>>{};
}
}
}
