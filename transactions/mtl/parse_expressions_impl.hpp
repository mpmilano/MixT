#pragma once
#include "parse_expressions_decl.hpp"
#include "common_strings.hpp"

namespace myria {
namespace mtl {

namespace parse_expressions {

using namespace mutils;

template <char op, char... str>
constexpr auto parse_binop(String<str...>)
{
  constexpr auto l = parse_expression(String<str...>::split(zero{}, String<op>{}).trim_ends());
  constexpr auto r = parse_expression(String<str...>::split(one{}, String<op>{}).trim_ends());
  return parse_phase::BinOp<op, std::decay_t<decltype(l)>, std::decay_t<decltype(r)>>{};
}

template <char... str>
constexpr bool
contains_operator()
{
  return String<str...>::contains_outside_parens(times_s{}) || String<str...>::contains_outside_parens(div_s{}) ||
         String<str...>::contains_outside_parens(minus_s{}) || String<str...>::contains_outside_parens(and_s{}) ||
         String<str...>::contains_outside_parens(or_s{}) || String<str...>::contains_outside_parens(lt_s{}) ||
         String<str...>::contains_outside_parens(gt_s{}) || String<str...>::contains_outside_parens(eq_s{}) ||
         String<str...>::contains_outside_parens(plus_s{});
}

template <char... str>
constexpr bool
contains_fieldref()
{
  return String<str...>::contains_outside_parens(String<'.'>{});
}

template <typename T>
auto strip_expression(parse_phase::Expression<T>)
{
  return T{};
}

// parens
template <char... str>
constexpr auto _parse_expression(std::enable_if_t<String<str...>::string[0] == '(', String<str...>>)
{
  return strip_expression(parse_expression(String<str...>::remove_first_char().remove_last_char()));
}

// variable reference
template <char... str>
constexpr auto _parse_expression(std::enable_if_t<!contains_operator<str...>() && !contains_fieldref<str...>() && !(String<str...>::string[0] == '(') &&
                                                    !String<str...>::trim_ends().contains(String<' '>{}) && !String<str...>::trim_ends().is_number(),
                                                  String<str...>>)
{
  return parse_phase::VarReference<std::decay_t<decltype(String<str...>::trim_ends())>>{};
}

// plain value
template <char... str>
constexpr auto _parse_expression(std::enable_if_t<!contains_operator<str...>() && !contains_fieldref<str...>() && !(String<str...>::string[0] == '(') &&
                                                    !String<str...>::trim_ends().contains(String<' '>{}) && String<str...>::trim_ends().is_number(),
                                                  String<str...>>)
{
  return parse_phase::Constant<String<str...>::trim_ends().parseInt()>{};
}

static_assert(contains_operator<'a', '.', 'b', ' ', '<', 'e'>());
// field reference
template <char... str>
constexpr auto _parse_expression(std::enable_if_t<!contains_operator<str...>() && contains_fieldref<str...>(), String<str...>>)
{
  using name = std::decay_t<decltype(parse_expression(String<str...>::split(zero{}, String<'.'>{}).trim_ends()))>;
  using field = std::decay_t<decltype(String<str...>::split(one{}, String<'.'>{}).trim_ends())>;
  return parse_phase::FieldReference<name, field>{};
}

//&& operator case
template <char... str>
constexpr auto _parse_expression(std::enable_if_t<String<str...>::contains_outside_parens(and_s{}), String<str...>>)
{
  return parse_binop<'&', str...>(String<str...>{});
}
//|| operator case
template <char... str>
constexpr auto _parse_expression(std::enable_if_t<String<str...>::contains_outside_parens(or_s{}), String<str...>>)
{
  return parse_binop<'|', str...>(String<str...>{});
}

//< operator case
template <char... str>
constexpr auto _parse_expression(std::enable_if_t<String<str...>::contains_outside_parens(lt_s{}), String<str...>>)
{
  return parse_binop<'<', str...>(String<str...>{});
}
//> operator case
template <char... str>
constexpr auto _parse_expression(std::enable_if_t<String<str...>::contains_outside_parens(gt_s{}), String<str...>>)
{
  return parse_binop<'>', str...>(String<str...>{});
}
//== operator case
template <char... str>
constexpr auto _parse_expression(std::enable_if_t<String<str...>::contains_outside_parens(eq_s{}), String<str...>>)
{
  return parse_binop<'=', str...>(String<str...>{});
}

//+ operator case
template <char... str>
constexpr auto _parse_expression(std::enable_if_t<String<str...>::contains_outside_parens(plus_s{}), String<str...>>)
{
  return parse_binop<'+', str...>(String<str...>{});
}
//* operator case
template <char... str>
constexpr auto _parse_expression(std::enable_if_t<String<str...>::contains_outside_parens(times_s{}), String<str...>>)
{
  return parse_binop<'*', str...>(String<str...>{});
}
// / operator case
template <char... str>
constexpr auto _parse_expression(std::enable_if_t<String<str...>::contains_outside_parens(div_s{}), String<str...>>)
{
  return parse_binop<'/', str...>(String<str...>{});
}
//- operator case
template <char... str>
constexpr auto _parse_expression(std::enable_if_t<String<str...>::contains_outside_parens(minus_s{}), String<str...>>)
{
  return parse_binop<'-', str...>(String<str...>{});
}

/*
template<char... str>
constexpr auto _parse_expression(String<str...>){
        return Expression<bool>{};
        }//*/
}

template <char... str>
constexpr auto parse_expression(mutils::String<str...>)
{
  using namespace parse_expressions;
  return parse_phase::Expression<std::decay_t<decltype(_parse_expression<str...>(String<str...>{}))>>{};
}
}
}
