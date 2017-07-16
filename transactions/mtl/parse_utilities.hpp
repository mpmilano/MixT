#pragma once

namespace myria {
namespace mtl {
namespace parse_utilities{

template <char c>
constexpr bool
is_method_char()
{
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_' || c == ')') || (c >= '0' && c <= '9');
}
    
template <char... str>
constexpr bool
contains_invocation(mutils::String<str...> = mutils::String<str...>{}, std::enable_if_t<!mutils::String<str...>::contains(mutils::String<'('>{})>* = nullptr)
{
  return false;
}

template <char... str>
constexpr bool
contains_invocation(mutils::String<str...> = mutils::String<str...>{}, std::enable_if_t<mutils::String<str...>::contains(mutils::String<'('>{})>* = nullptr)
{
  constexpr auto paren_group = mutils::String<str...>::template next_paren_group<'(', ')'>();
  constexpr char c = DECT(paren_group)::pre::reverse().trim_ends().string[0];
  return is_method_char<c>();
}

  using skip = parse_phase::Statement<parse_phase::Sequence<> >;

static_assert(!contains_invocation<'3'>());

}}}
