#pragma once
#include "mutils/CTString.hpp"
#include "mtl/AST_parse.hpp"

namespace myria {
namespace mtl {
namespace parse_utilities {

template <char c>
constexpr bool
is_method_char()
{
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_' || c == ')') || (c >= '0' && c <= '9');
}

constexpr bool contains_invocation(mutils::String<>)
{
  return false;
}

template <char c1, char... str>
constexpr bool
contains_invocation(mutils::String<c1, str...> = mutils::String<c1, str...>{},
                    std::enable_if_t<!mutils::String<c1, str...>::contains(mutils::String<'('>{})>* = nullptr)
{
  return false;
}

template <char c1, char... str>
constexpr bool
contains_invocation(mutils::String<c1, str...> = mutils::String<c1, str...>{},
                    std::enable_if_t<mutils::String<c1, str...>::contains(mutils::String<'('>{})>* = nullptr)
{
  constexpr auto paren_group = mutils::String<c1, str...>::template next_paren_group<'(', ')'>();
  constexpr char c = DECT(paren_group)::pre::reverse().trim_ends().string[0];
  return is_method_char<c>() || contains_invocation(typename DECT(paren_group)::post{});
}

using skip = parse_phase::Statement<parse_phase::Sequence<>>;

template <char lp, char rp, char c, char... str>
constexpr bool
_is_paren_group(mutils::String<c, str...> = mutils::String<c, str...>{}, std::enable_if_t<c != lp>* = nullptr)
{
  return false;
}

template <char lp, char rp, char... str>
constexpr bool
_is_paren_group(mutils::String<lp, str...> = mutils::String<lp, str...>{})
{
  using pgp = DECT(mutils::String<lp, str...>::template next_paren_group<lp, rp>());
  return pgp::pre::trim_ends() == pgp::post::trim_ends() && pgp::pre::trim_ends() == mutils::String<>{};
}

template <char lp, char rp, char... str>
constexpr bool
is_paren_group(mutils::String<str...> a = mutils::String<str...>{})
{
  return _is_paren_group<lp, rp>(a);
}

static_assert(is_paren_group<'(', ')'>(mutils::String<'(', 'h', 'e', 'l', 'l', 'o', ')'>{}));

static_assert(is_paren_group<'(', ')', '(', ')'>());
static_assert(!is_paren_group<'(', ')', 'a', '(', ')'>());
static_assert(!is_paren_group<'(', ')', ' ', '(', ')'>());

static_assert(!contains_invocation<'3'>());
}
}
}
