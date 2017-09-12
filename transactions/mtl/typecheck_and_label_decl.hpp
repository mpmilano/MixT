#pragma once
#include "mtl/CTString.hpp"
#include "mtl/mtlutils.hpp"
#include "mtl/AST_parse.hpp"
#include "mtl/AST_typecheck.hpp"

namespace myria {
namespace mtl {

template <typename T, typename l, type_location loc, char... str>
constexpr auto binding_label(type_binding<mutils::String<str...>, T, Label<l>, loc>)
{
  return Label<l>{};
}

template <typename>
struct is_type_binding;
template <typename Var, typename Val, typename lvl, type_location loc>
struct is_type_binding<type_binding<Var, Val, lvl, loc>> : public std::true_type
{
};
template <typename>
struct is_type_binding : public std::false_type
{
};

namespace typecheck_phase {

template <typename Level, typename... Bindings>
struct type_environment;

template <typename l, typename... Bindings>
struct type_environment<Label<l>, Bindings...>
{

  using pc_label = Label<l>;

  static_assert(mutils::forall<is_type_binding<Bindings>::value...>());
  template <char... str>
  static constexpr auto get_binding(mutils::String<str...>)
  {
    using name = mutils::String<str...>;
    return DECT(*mutils::find_match<DECT(Bindings::template get_binding<name>(name{}))...>()){};
  }
  template<typename new_label, typename... more_bindings>
  static constexpr auto extend(){
    return type_environment<new_label, Bindings..., more_bindings...>{};
  }
};

template <typename T, typename... Bindings>
constexpr auto typecheck(type_environment<Bindings...>, parse_phase::Statement<T>);
template <typename T, typename... Bindings>
constexpr auto typecheck(type_environment<Bindings...>, parse_phase::Expression<T>);
/*
template <typename Var, typename Val, typename... Bindings>
constexpr auto typecheck(type_environment<Bindings...>,
parse_phase::Binding<Var, Val>);//*/
}
}
}
