#pragma once

#include "ensdorsement.hpp"
#include "AST_split.hpp"

namespace myria {
namespace mtl {
namespace parse_phase {

template <typename AST>
constexpr auto split_endorse(AST a);

template <typename l, typename y, typename v, typename e>
constexpr auto _split_endorse(Binding<l, y, v, e>)
{
}

template <typename l, typename y, typename s, typename f>
constexpr auto _split_endorse(Expression<l, y, FieldReference<s, f>>)
{
}

template <typename l, typename y, typename v>
constexpr auto _split_endorse(Expression<l, y, VarReference<v>>)
{
}

template <typename l, int i>
constexpr auto _split_endorse(Expression<int, y, Constant<i>>)
{
}

template <typename l, typename y, char op, typename L, typename R>
constexpr auto _split_endorse(Expression<l, y, BinOp<op, L, R>>)
{
}

  //expressions just don't worry about splitting.  They don't have a pre_ or post_, but just a normal pack
  
template <typename b, typename e, typename s>
constexpr auto _split_endorse(Statement<Let<Binding<b,e>, s> >)
{
  using res = DECT(split_endorse(s));
  auto pre_endorse_ast = typename res::pre_endorse;
  auto pre_endorse_env = typename res::pre_endorse_env;
  auto post_endorse_ast = typename res::post_endorse;
  auto post_endorse_env = typename res::post_endorse_env;
  using rese = DECT(split_endorse(e));
  auto aste = typename rese::ast;
  auto enve = typename rese::env;
  constexpr bool is_pre = pre_endorse_env::contains_use(b);
  using new_ast = Statement<Let<Binding<b,aste>, std::conditional_t<is_pre, pre_endorse_ast, post_endorse_ast> > >;
  return statement_return<
    std::conditional_t<is_pre, new_ast, pre_endorse_ast>,
    std::conditional_t<is_pre, DECT(pre_endorse_env::combine(enve)), pre_endorse_env>,
    std::conditional_t<!is_pre, new_ast, post_endorse_ast>,
    std::conditional_t<!is_pre, DECT(post_endorse_env::combine(enve)), post_endorse_env>,
    >{};
}

template <typename l, typename b, typename e>
constexpr auto _split_endorse(Statement<l, LetRemote<b, e>>)
{
}

	template <typename l, typename n, typename h, typename e>
	constexpr auto _split_endorse(Statement<l, LetIsValid<n,h, e>>)
{
}

template <typename l, typename c, typename t, typename e>
constexpr auto _split_endorse(Statement<l, If<c, t, e>>)
{
  using resc = DECT(split_endorse(c));
  auto astc = typename resc::ast;
  auto envc = typename resc::env;
  using rest = DECT(split_endorse(t));
  auto pre_endorse_astt = typename rest::pre_endorse;
  auto pre_endorse_envt = typename rest::pre_endorse_env;
  auto post_endorse_astt = typename rest::post_endorse;
  auto post_endorse_envt = typename rest::post_endorse_env;
  using rese = DECT(split_endorse(e));
  auto pre_endorse_aste = typename rese::pre_endorse;
  auto pre_endorse_enve = typename rese::pre_endorse_env;
  auto post_endorse_aste = typename rese::post_endorse;
  auto post_endorse_enve = typename rese::post_endorse_env;
  constexpr bool all_pre_empty = is_empty_ast<pre_endorse_astt>::value && is_empty_ast<pre_endorse_aste>::value;
  constexpr bool all_post_empty = is_empty_ast<post_endorse_astt>::value && is_empty_ast<post_endorse_aste>::value;
  return statement_return<
    std::conditional_t<all_pre_empty, Statement<Sequence<> >, Statement<If<astc,pre_endorse_astt, pre_endorse_aste> > >,
    std::conditional_t<all_pre_empty, DECT(pre_endorse_envt::combine(pre_endorse_enve{})), DECT(pre_endorse_envt::combine(pre_endorse_enve::combine(envc)))>,
    std::conditional_t<all_post_empty, Statement<Sequence<> >, Statement<If<astc,post_endorse_astt, post_endorse_aste> > >,
    std::conditional_t<all_post_empty, DECT(post_endorse_envt::combine(post_endorse_enve{})), DECT(post_endorse_envt::combine(post_endorse_enve::combine(envc)))>
    >{};
}

template <typename l, typename c, typename e>
constexpr auto _split_endorse(Statement<l, While<c, e>>)
{
}

template <typename l, typename... seq>
constexpr auto _split_endorse(Statement<l, Sequence<seq...>>)
{
}

template <typename AST>
constexpr auto split_endorse(AST a)
{
  return _split_endorse(a);
}
}
}
}
