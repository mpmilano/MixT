#pragma once

namespace mutils {
namespace CTString {

	
	template<typename SoFar, typename Remains>
	struct then_case_str{
		using first = SoFar;
    using second = Remains;
	};

	
template <typename SoFar, typename SplitOn>
static constexpr auto split(String<>)
{
  return then_case_str<SoFar,String<> >{};
}

template <typename SoFar, typename SplitOn, typename Remains>
static constexpr auto split(Remains);

// paren group skip case (
template <typename SoFar, typename SplitOn, char... str2>
static constexpr auto split(String<'(', str2...>);

// paren group skip case {
template <typename SoFar, typename SplitOn, char... str2>
static constexpr auto split(String<'{', str2...>);

// paren group skip case
template <typename SoFar, typename SplitOn, char lparen, char rparen, char... str2>
static constexpr auto split(String<lparen, str2...>)
{
  using paren_group = std::decay_t<decltype(String<lparen, str2...>::template strip_paren_group<lparen, rparen>(zero{}))>;
  using after_paren = std::decay_t<decltype(String<lparen, str2...>::template strip_paren_group<lparen, rparen>(one{}))>;
  using sofar = std::decay_t<decltype(SoFar::append(paren_group{}))>;
  return split<sofar, SplitOn>(after_paren{});
}

// paren group skip case (
template <typename SoFar, typename SplitOn, char... str2>
static constexpr auto split(String<'(', str2...>)
{
  return split<SoFar, SplitOn, '(', ')', str2...>(String<'(', str2...>{});
}

// paren group skip case {
template <typename SoFar, typename SplitOn, char... str2>
static constexpr auto split(String<'{', str2...>)
{
  return split<SoFar, SplitOn, '{', '}', str2...>(String<'{', str2...>{});
}
	
template <typename SoFar, typename SplitOn, typename Remains>
static constexpr auto split(Remains)
{
  static_assert(SoFar{}.string[(SoFar::string_length == 0 ? 0 : SoFar::string_length - 1)] != '(' &&
                SoFar{}.string[(SoFar::string_length == 0 ? 0 : SoFar::string_length - 1)] != '{');
	using then_case = then_case_str<SoFar,Remains>;
  using split_case_sofar = std::decay_t<decltype(SoFar::append(Remains::first_char()))>;
  return std::conditional_t<
    /*if */ Remains::begins_with(SplitOn{}),
    /* then */ then_case,
    /* else */ std::decay_t<decltype(split<split_case_sofar, SplitOn>(Remains::remove_first_char()))>>{};
}
}
}
