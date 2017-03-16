#pragma once

#include "CTString_decl.hpp"
#include "CTString_split.hpp"
#include "CTString_strip_paren_group.hpp"
#include "utils.hpp"

namespace mutils {

// empty case

constexpr bool String<>::operator==(String<>) const
{
  return true;
}

template <char c, char... str>
constexpr bool String<>::operator==(String<c, str...>) const
{
  return false;
}

template <char... str2>
constexpr bool String<>::begins_with(String<str2...>)
{
  return sizeof...(str2) == 0;
}

constexpr String<> String<>::without_prefix(String<>)
{
  return String{};
}

template <char c1, char... str2>
constexpr auto String<>::without_prefix(String<c1, str2...>)
{
  static_assert(sizeof...(str2) == -1, "Error: cannot remove from empty string");
  return String{};
}

template <char... str2>
constexpr bool String<>::ends_with(String<str2...>)
{
  return sizeof...(str2) == 0;
}

template <char... str2>
constexpr String<str2...> String<>::append(String<str2...>)
{
  return String<str2...>{};
}

constexpr auto String<>::reverse()
{
  return String<>{};
}

template <char... str2>
constexpr auto String<>::without_suffix(String<str2...>)
{
  if (sizeof...(str2) == 0)
    return String<>{};
  else
    assert(false && "Error: cannot remove suffix from empty string");
}

constexpr auto String<>::trim_ends()
{
  return String<>{};
}

template <char... str2>
constexpr String<> String<>::split(zero, String<str2...>)
{
  return String<>{};
}

constexpr bool
String<>::is_number()
{
  return false;
}

// non-empty case

template <char... str>
template <char... str2>
constexpr bool String<str...>::operator==(String<str2...>) const
{
  return sizeof...(str) == sizeof...(str2) && String::begins_with(String<str2...>{});
}

namespace CTString {

constexpr int
length(char const* const arr)
{
  for (int rlen = 0; rlen < 4000; ++rlen) {
    if (arr[rlen] == 0)
      return rlen;
  }
  return -1;
}

constexpr bool
begins_with(char const* const this_str, char const* const other)
{
  auto length_other = length(other);
  auto length_this = length(this_str);
  if (length_other > length_this)
    return false;
  int limit = length_other;
  if (limit == 0)
    return true;
  else
    for (int i = 0; i < limit; ++i)
      if (this_str[i] != other[i])
        return false;
  return true;
}
}

template <char... str>
template <char... str2>
constexpr bool String<str...>::begins_with(String<str2...>)
{
  return CTString::begins_with(String<str...>::string, String<str2...>::string);
}

namespace CTString {
template <char c, char... str2>
constexpr String<str2...> remove_first_char(String<c, str2...>)
{
  return String<str2...>{};
}
}

template <char... str>
constexpr auto String<str...>::remove_first_char()
{
  return CTString::remove_first_char(String{});
}

template <char... str>
constexpr auto String<str...>::remove_last_char()
{
  return reverse().remove_first_char().reverse();
}

namespace CTString {
template <char c, char... str2>
constexpr String<c> first_char(String<c, str2...>)
{
  return String<c>{};
}
}

template <char... str>
constexpr auto String<str...>::first_char()
{
  return CTString::first_char(String{});
}

template <char... str>
constexpr String<str...> String<str...>::without_prefix(String<>)
{
  return String{};
}

template <char... str>
template <char c1, char... str2>
constexpr auto String<str...>::without_prefix(String<c1, str2...>)
{
  constexpr bool assert_this = begins_with(String<c1, str2...>{});
  if (!assert_this) {
    assert(assert_this && "failed: string does not begin with this prefix");
  }
  return remove_first_char().without_prefix(String<str2...>{});
}

template <char... str>
template <char... str2>
constexpr bool String<str...>::ends_with(String<str2...>)
{
  constexpr int limit = sizeof...(str2);
  if (limit == 0)
    return true;
  else {
    int j = 0;
    for (int i = string_length - limit; i < string_length; (++i, ++j))
      if (String<str2...>::string[j] != string[i])
        return false;
  }
  return true;
}

template <char... str>
template <char... str2>
constexpr bool String<str...>::contains(String<str2...>)
{
  constexpr auto* my_str = String<str...>::string;
  constexpr auto* other_str = String<str2...>::string;
  for (int index = 0; index < sizeof...(str); ++index) {
    if (my_str[index] == other_str[0] && CTString::begins_with(my_str + index, other_str))
      return true;
  }
  return false;
}

template <char... str>
template <char... str2>
constexpr bool String<str...>::contains_outside_parens(String<str2...>)
{
  int paren_count = 0;
  int bracket_count = 0;
  static_assert(String<str2...>::string[0] != '(' && String<str2...>::string[0] != '{');
  constexpr auto* my_str = String<str...>::string;
  constexpr auto* other_str = String<str2...>::string;
  for (int index = 0; index < sizeof...(str); ++index) {
    if (my_str[index] == '(')
      ++paren_count;
    if (my_str[index] == '{')
      ++bracket_count;
    if (my_str[index] == ')' && paren_count > 0)
      --paren_count;
    if (my_str[index] == '}' && bracket_count > 0)
      --bracket_count;
    if (my_str[index] == other_str[0] && (paren_count == 0) && (bracket_count == 0) && CTString::begins_with(my_str + index, other_str))
      return true;
  }
  return false;
}

template <char... str>
template <char... str2>
constexpr String<str..., str2...> String<str...>::append(String<str2...>)
{
  return String<str..., str2...>{};
}

namespace CTString {
template <char c1, char... str2>
constexpr auto reverse_helper()
{
  return String<str2...>::reverse().append(String<c1>{});
}
}

template <char... str>
constexpr auto String<str...>::reverse()
{
  return CTString::reverse_helper<str...>();
}

template <char... str>
template <char... str2>
constexpr auto String<str...>::without_suffix(String<str2...>)
{
  return reverse().without_prefix(String<str2...>::reverse()).reverse();
}
namespace CTString {
template <char... str2>
constexpr auto trim_ends_helper(String<' ', str2...>)
{
  return String<str2...>::trim_ends();
}

// ends with space, trust me on this
template <char... str2>
constexpr auto trim_ends_helper2(std::true_type*, String<str2...>)
{
  return String<str2...>::without_suffix(String<' '>{}).trim_ends();
}

template <char... str2>
constexpr auto trim_ends_helper2(std::false_type*, String<str2...>)
{
  return String<str2...>{};
}

template <char... str2>
constexpr auto trim_ends_helper(String<str2...>)
{
  std::integral_constant<bool, String<str2...>::ends_with(String<' '>{})>* choice{ nullptr };
  return trim_ends_helper2(choice, String<str2...>{});
}
}

template <char... str>
constexpr auto String<str...>::trim_ends()
{
  return CTString::trim_ends_helper(String{});
}

template <char... str>
template <char... str2>
constexpr auto String<str...>::split(zero, String<str2...>)
{
  // not allowed to split on paren groups for now, sorry.
  static_assert(!(String<str2...>::begins_with(String<'('>{}) || String<str2...>::begins_with(String<'{'>{})));
  using pair = std::decay_t<decltype(CTString::split<String<>, String<str2...>>(String{}))>;
  return typename pair::first{};
}

template <char... str>
template <char... str2>
constexpr auto String<str...>::after_fst(String<str2...>)
{
  using pair = std::decay_t<decltype(CTString::split<String<>, String<str2...>>(String{}))>;
  static_assert(!std::is_same<typename pair::second, String<>>::value, "Error: cannot split beyond end of string");
  return pair::second::without_prefix(String<str2...>{});
}

template <char... str>
template <typename n, char... str2>
constexpr auto String<str...>::split(succ<n>, String<str2...>)
{
  return after_fst(String<str2...>{}).split(n{}, String<str2...>{});
}

template <char... str>
template <char lparen, char rparen, typename n>
constexpr auto String<str...>::strip_paren_group(n)
{
  using namespace CTString;
  return strip_paren_group_struct<lparen, rparen>::strip_paren_group_start(n{}, String{});
}

template <char... str>
constexpr bool
String<str...>::is_number()
{
  for (int i = 0; i < string_length; ++i) {
    if (!isDigit(string[i]))
      return false;
  }
  return true;
}

template <char... str>
constexpr long long
String<str...>::parseInt()
{
  long long multiplier = exp(10, string_length - 1);
  long long accum = 0;
  for (int i = 0; i < string_length; ++i) {
    accum += toInt(string[i]) * multiplier;
    multiplier /= 10;
  }
  return accum;
}

template <char... str>
constexpr const char String<str...>::string[];
template <char... str>
constexpr const decltype(sizeof...(str)) String<str...>::string_length;
template <char... str>
constexpr String<str...> const* const String<str...>::p;
template <char... str>
constexpr const as_value<String<str...>> String<str...>::v;

template <typename>
struct is_string : public std::false_type
{
};
template <char... str>
struct is_string<String<str...>> : public std::true_type
{
};

template <char... str>
std::ostream& operator<<(std::ostream& o, const String<str...>&)
{
  return o << String<str...>::string;
}

template <char... str>
struct string_of<String<str...>>
{
  std::string value{ String<str...>::string };
};
}
