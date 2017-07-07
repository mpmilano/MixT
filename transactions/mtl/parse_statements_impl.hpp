#pragma once
#include "common_strings.hpp"
#include "parse_bindings.hpp"
#include "parse_expressions_decl.hpp"
#include "parse_statements_decl.hpp"
#include "parse_utilities.hpp"

namespace myria {
namespace mtl {

namespace parse_statements {

using namespace mutils;

template <char... _str>
constexpr auto parse_let(String<'v', 'a', 'r', ' ', _str...>)
{
  constexpr auto binding = String<_str...>::split(zero{}, comma_s{}).trim_ends();
  constexpr auto body_str = String<'{'>::append(String<_str...>::after_fst(String<','>{}).trim_ends()).append(String<'}'>{});
  ;
  return parse_phase::Let<std::decay_t<decltype(parse_binding(binding))>, std::decay_t<decltype(parse_statement(body_str))>>{};
}

template <char... _str>
constexpr auto parse_let_remote(String<'r', 'e', 'm', 'o', 't', 'e', ' ', _str...>)
{
  constexpr auto binding = String<_str...>::split(zero{}, comma_s{}).trim_ends();
  constexpr auto body_str = String<'{'>::append(String<_str...>::after_fst(String<','>{}).trim_ends()).append(String<'}'>{});
  return parse_phase::LetRemote<std::decay_t<decltype(parse_binding(binding))>, std::decay_t<decltype(parse_statement(body_str))>>{};
}

template <char... _str>
constexpr auto parse_if(String<'i', 'f', _str...>)
{
  static_assert(String<_str...>::trim_ends().string[0] == '(');
  constexpr auto condition_s = String<_str...>::trim_ends().template strip_paren_group<'(', ')'>(zero{}).trim_ends();
  constexpr auto remainder_s = String<_str...>::trim_ends().template strip_paren_group<'(', ')'>(one{}).trim_ends();
  constexpr auto then_s = remainder_s.trim_ends().template strip_paren_group<'{', '}'>(zero{}).trim_ends();
  constexpr auto else_s = remainder_s.trim_ends()
                            .template strip_paren_group<'{', '}'>(one{})
                            .trim_ends()
                            .split(one{}, MUTILS_STRING(else){})
                            .trim_ends()
                            .template strip_paren_group<'{', '}'>(zero{})
                            .trim_ends();
  return parse_phase::If<std::decay_t<decltype(parse_expression(condition_s))>, std::decay_t<decltype(parse_statement(then_s))>,
                         std::decay_t<decltype(parse_statement(else_s))>>{};
}

template <char... str>
constexpr auto parse_return(String<'r', 'e', 't', 'u', 'r', 'n', str...>)
{
  return parse_phase::Return<DECT(parse_expression(String<str...>{}))>{};
}

template <char... _str>
constexpr auto parse_while(String<'w', 'h', 'i', 'l', 'e', _str...>)
{
  using str = std::decay_t<decltype(String<_str...>::trim_ends())>;
  static_assert(str::trim_ends().string[0] == '(');
  constexpr auto condition_s = str::template strip_paren_group<'(', ')'>(zero{}).trim_ends();
  constexpr auto remainder_s = str::template strip_paren_group<'(', ')'>(one{}).trim_ends();
  constexpr auto body_s = remainder_s.template strip_paren_group<'{', '}'>(zero{}).trim_ends();
  return parse_phase::While<std::decay_t<decltype(parse_expression(condition_s))>, std::decay_t<decltype(parse_statement(body_s))>>{};
}


  template <typename Name, typename Hndl, typename... args>
  constexpr auto operation_as_statement(parse_phase::Expression<parse_phase::Operation<Name,Hndl,args...>>){
    return parse_phase::LetOperation<String<0>,Name,Hndl,parse_utilities::skip, args...>{};
  }
  
// strip whitespace
template <char... _str>
constexpr auto
_parse_statement(std::enable_if_t<String<_str...>::begins_with(space_s{}) || String<_str...>::ends_with(space_s{}), String<_str...>>)
{
  using str = String<_str...>;
  return parse_statement(str::trim_ends());
}

/* Retiring: brackets indicate sequence, now handled by sequence case.
//strip brackets
template<char... _str>
constexpr auto _parse_statement(
        std::enable_if_t<String<_str...>::begins_with(lbracket_s{}),
        String<_str...> > ){
        using str = String<_str...>;
        if (str::ends_with(rbracket_s{}))
                return parse_statement(str::without_prefix(lbracket_s{}).without_suffix(rbracket_s{}));
        else assert(false && "Error: bracket mismatch!");
}//*/

// sequence case;

template <typename Seq1, typename... Seq2>
constexpr auto
append_sequences(parse_phase::Statement<Seq1>, parse_phase::Statement<parse_phase::Sequence<Seq2...>>)
{
  return Seq1::append(parse_phase::Sequence<Seq2...>{});
}

template <typename Seq1, typename binding, typename body>
constexpr auto append_sequences(parse_phase::Statement<Seq1>, parse_phase::Statement<parse_phase::Let<binding, body>>)
{
  using namespace parse_phase;
  return Seq1::append(Sequence<Statement<Let<binding, body>>>{});
}

template <typename Seq1, typename binding, typename body>
constexpr auto append_sequences(parse_phase::Statement<Seq1>, parse_phase::Statement<parse_phase::LetRemote<binding, body>>)
{
  using namespace parse_phase;
  return Seq1::append(Sequence<Statement<LetRemote<binding, body>>>{});
}

template <char... _str>
constexpr auto
_parse_statement(std::enable_if_t<String<_str...>::begins_with(String<'{'>{}) && String<_str...>::remove_first_char().contains_outside_parens(String<','>{}) &&
                                    !(String<_str...>::remove_first_char().trim_ends().begins_with(var_s{}) ||
                                      String<_str...>::remove_first_char().trim_ends().begins_with(remote_s{})),
                                  String<_str...>>)
{
  static_assert(String<_str...>::ends_with(String<'}'>{}), "Error: bracket mismatch!");
  using str = std::decay_t<decltype(String<_str...>::remove_first_char().trim_ends())>;
  using fst = std::decay_t<decltype(parse_statement(str::split(zero{}, String<','>{}).trim_ends()))>;
  using rst = std::decay_t<decltype(str::after_fst(String<','>{}).trim_ends())>;
  return append_sequences(parse_phase::Statement<parse_phase::Sequence<fst>>{}, parse_statement(String<'{'>::append(rst{})));
}

// singleton sequence case;
template <char... _str>
constexpr auto
_parse_statement(std::enable_if_t<String<_str...>::begins_with(String<'{'>{}) && !String<_str...>::remove_first_char().contains_outside_parens(String<','>{}) &&
                                    !(String<_str...>::remove_first_char().trim_ends() == String<'}'>{}),
                                  String<_str...>>)
{
  using str = std::decay_t<decltype(String<_str...>::trim_ends())>;
  static_assert(str::ends_with(String<'}'>{}), "Error: bracket mismatch!");
  using stmt = std::decay_t<decltype(parse_statement(str::remove_first_char().remove_last_char().trim_ends()))>;
  return parse_phase::Sequence<stmt>{};
}

// empty sequence case;
template <char... _str>
constexpr auto
_parse_statement(
  std::enable_if_t<String<_str...>::begins_with(String<'{'>{}) && String<_str...>::remove_first_char().trim_ends() == String<'}'>{}, String<_str...>>)
{
  return parse_phase::Sequence<>{};
}

// let
template <char... _str>
constexpr auto
_parse_statement(std::enable_if_t<String<_str...>::begins_with(var_s{}), String<_str...>>)
{
  using str = String<_str...>;
  return parse_let(str::trim_ends());
}

// let_remote
template <char... _str>
constexpr auto
_parse_statement(std::enable_if_t<String<_str...>::begins_with(remote_s{}), String<_str...>>)
{
  using str = String<_str...>;
  return parse_let_remote(str::trim_ends());
}

// bindings in curlies

template <typename T>
auto remove_statement(parse_phase::Statement<T>)
{
  return T{};
}

template <char... _str>
constexpr auto
_parse_statement(std::enable_if_t<String<_str...>::begins_with(String<'{'>{}) && (String<_str...>::remove_first_char().trim_ends().begins_with(var_s{}) ||
                                                                                  String<_str...>::remove_first_char().trim_ends().begins_with(remote_s{})),
                                  String<_str...>>)
{
  static_assert(String<_str...>::ends_with(String<'}'>{}), "Error: bracket mismatch!");
  return remove_statement(parse_statement(String<_str...>::remove_last_char().remove_first_char().trim_ends()));
}

// while
template <char... _str>
constexpr auto
_parse_statement(std::enable_if_t<String<_str...>::begins_with(while_s{}), String<_str...>>)
{
  using str = String<_str...>;
  return parse_while(str::trim_ends());
}

// if
template <char... _str>
constexpr auto
_parse_statement(std::enable_if_t<String<_str...>::begins_with(if_s{}), String<_str...>>)
{
  using str = String<_str...>;
  return parse_if(str::trim_ends());
}

// return
template <char... _str>
constexpr auto
_parse_statement(std::enable_if_t<String<_str...>::begins_with(return_s{}), String<_str...>>)
{
  using str = String<_str...>;
  return parse_return(str::trim_ends());
}

// assignment
template <char... _str>
constexpr auto
_parse_statement(std::enable_if_t<!(String<_str...>::begins_with(String<'{'>{}) || String<_str...>::begins_with(remote_s{}) ||
                                    String<_str...>::begins_with(var_s{}) || String<_str...>::begins_with(while_s{}) || String<_str...>::begins_with(if_s{}) ||
                                    String<_str...>::contains_outside_parens(comma_s{}) || String<_str...>::begins_with(return_s{})) &&
                                    String<_str...>::contains_outside_parens(String<'='>{}),
                                  String<_str...>>)
{
  static_assert(!String<_str...>::contains_outside_parens(comma_s{}));
  using str = String<_str...>;
  auto var = parse_expression(str::split(zero{}, String<'='>{}).trim_ends());
  auto expr = parse_expression(str::split(one{}, String<'='>{}).trim_ends());
  return parse_phase::Assignment<std::decay_t<decltype(var)>, std::decay_t<decltype(expr)>>{};
}

// top-level operation
template <char... _str>
constexpr auto
_parse_statement(
  std::enable_if_t<!(String<_str...>::begins_with(String<'{'>{}) || String<_str...>::begins_with(remote_s{}) || String<_str...>::begins_with(var_s{}) ||
                     String<_str...>::begins_with(while_s{}) || String<_str...>::begins_with(if_s{}) || String<_str...>::contains_outside_parens(comma_s{}) ||
                     String<_str...>::begins_with(return_s{}) || String<_str...>::contains_outside_parens(String<'='>{})) &&
  parse_utilities::contains_invocation<_str...>(),
                   String<_str...>>)
{
  static_assert(!String<_str...>::contains_outside_parens(String<'.'>{}));
  return operation_as_statement(parse_expression(String<_str...>::trim_ends()));
}

template <char... str>
constexpr auto
parse_statement_trimmed(String<str...>)
{
  return parse_phase::Statement<std::decay_t<decltype(_parse_statement<str...>(String<str...>{}))>>{};
}
}

template <char... str>
constexpr auto
parse_statement(mutils::String<str...>)
{
  using namespace parse_statements;
  return parse_statement_trimmed(String<str...>::trim_ends());
}
}
}
