#pragma once
#include <type_traits>
#include <sstream>
#include <cxxabi.h>
#include <unistd.h>
#include <cassert>
#include "as_value.hpp"
#include "macro_utils.hpp"
#include "peano.hpp"

namespace mutils {

template <char... str>
struct String;

template <>
struct String<>
{
  constexpr String() = default;
  static const constexpr char string[] = { 0 };
  static const constexpr std::size_t string_length = 0;
  static constexpr String const* const p{ nullptr };
  static const constexpr as_value<String> v{};

  constexpr bool operator==(String<>) const;

  template <char c, char... str>
  constexpr bool operator==(String<c, str...>) const;

  template <char... str2>
  static constexpr bool begins_with(String<str2...>);

  static constexpr String without_prefix(String<>);

  template <char c1, char... str2>
  static constexpr auto without_prefix(String<c1, str2...>);

  template <char... str2>
  static constexpr bool ends_with(String<str2...>);

  template <char... str2>
  static constexpr String<str2...> append(String<str2...>);

  static constexpr auto reverse();

  template <char... str2>
  static constexpr auto without_suffix(String<str2...>);

  static constexpr auto trim_ends();

  template <char... str2>
  static constexpr String split(zero, String<str2...>);

  static constexpr bool is_number();
};

template <char... str>
struct String
{
  constexpr String() = default;
  static const constexpr char string[] = { str..., 0 };
  static const constexpr decltype(sizeof...(str)) string_length = sizeof...(str);
  static constexpr String const* const p{ nullptr };
  static const constexpr as_value<String> v{};

  template <char... str2>
  constexpr bool operator==(String<str2...>) const;

  template <char... str2>
  static constexpr bool begins_with(String<str2...>);

  static constexpr auto remove_first_char();

  static constexpr auto remove_last_char();

  static constexpr auto first_char();

  static constexpr String without_prefix(String<>);

  template <char c1, char... str2>
  static constexpr auto without_prefix(String<c1, str2...>);

  template <char... str2>
  static constexpr bool ends_with(String<str2...>);

  template <char... str2>
  static constexpr bool contains(String<str2...>);

  template <char... str2>
  static constexpr bool contains_outside_parens(String<str2...>);

  template <char... str2>
  static constexpr String<str..., str2...> append(String<str2...>);

  static constexpr auto reverse();

  template <char... str2>
  static constexpr auto without_suffix(String<str2...>);

  static constexpr auto trim_ends();

  template <char... str2>
  static constexpr auto split(zero, String<str2...>);

  template <typename n, char... str2>
  static constexpr auto split(succ<n>, String<str2...>);

  template <char... str2>
  static constexpr auto after_fst(String<str2...>);

  template <char lparen, char rparen, typename n>
  static constexpr auto strip_paren_group(n);

  static constexpr bool is_number();

  static constexpr int parseInt();
};

template <char... str>
std::ostream& operator<<(std::ostream& o, const String<str...>&);
}
