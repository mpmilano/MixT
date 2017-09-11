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
  const char string[1] = { 0 };
  static const constexpr std::size_t string_length = 0;
  static constexpr String const* const p{ nullptr };
  static const constexpr as_value<String> v{};
	using name = String;

  constexpr bool operator==(String<>) const;

  template <char c, char... str>
  constexpr bool operator==(String<c, str...>) const;

  template <char... str2>
  constexpr bool operator!=(String<str2...>) const{
    return !(String{} == String<str2...>{});
  }

  template <char... str2>
  static constexpr bool begins_with(String<str2...>);

  static constexpr String without_prefix(String<>);

  template <char c1, char... str2>
  static constexpr auto without_prefix(String<c1, str2...>);

  template <char... str2>
  static constexpr bool ends_with(String<str2...>);

	static constexpr bool contains(String<>){return true;}
	
	template<char c1, char... str2>
	static constexpr bool contains(String<c1,str2...>){return false;}

  template <char... str2>
  static constexpr String<str2...> append(String<str2...>);

	template <char... str2>
  static constexpr String<str2...> append();

	template <typename s1, typename s2, typename... str2>
  static constexpr auto append(s1,s2,str2...);

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
  const char string[sizeof...(str) + 1] = { str..., 0 };
  static const constexpr decltype(sizeof...(str)) string_length = sizeof...(str);
  static constexpr String const* const p{ nullptr };
  static const constexpr as_value<String> v{};
	using name = String;

  template <char... str2>
  constexpr bool operator==(String<str2...>) const;

  template <char... str2>
  constexpr bool operator!=(String<str2...>) const{
    return !(String{} == String<str2...>{});
  }

  template <char... str2>
  static constexpr bool begins_with(String<str2...>);

  static constexpr auto remove_first_char();

  static constexpr auto remove_last_char();

  static constexpr auto first_char();

  static constexpr auto last_char();

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

	template <char... str2>
  static constexpr String<str..., str2...> append();
	
	template <typename s1, typename s2, typename... str2>
  static constexpr auto append(s1,s2,str2...);

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

  template <char... str2>
  static constexpr auto before_lst(String<str2...>);
  
  template <char... str2>
  static constexpr auto after_lst(String<str2...>);

  template <char lparen, char rparen, typename n>
  static constexpr auto strip_paren_group(n);

	template <char lparen, char rparen>
  static constexpr auto next_paren_group();

  static constexpr bool is_number();

  static constexpr int parseInt();
};

	template<long long i>
	constexpr auto string_from_int();

template <char... str>
std::ostream& operator<<(std::ostream& o, const String<str...>&);
}
