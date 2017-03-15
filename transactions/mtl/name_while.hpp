#pragma once

#include "AST_typecheck.hpp"
#include "utils.hpp"

namespace myria {
namespace mtl {
namespace typecheck_phase {

template <char seqnum, char depth, typename AST>
constexpr auto name_while(AST a);

// expressions
template <char, char, typename l, typename yields, typename expr>
constexpr auto _name_while(Expression<l, yields, expr> a)
{
  return a;
}

// binding
template <char, char, typename l, typename yields, typename name, typename expr>
constexpr auto _name_while(Binding<l, yields, name, expr> a)
{
  return a;
}

// statements
template <char seqnum, char depth, typename l, typename binding, typename body>
constexpr auto _name_while(Statement<l, Let<binding, body>>)
{
  return Statement<l, Let<binding, DECT(name_while<seqnum, depth>(body{}))>>{};
}

template <char seqnum, char depth, typename l, typename binding, typename body>
constexpr auto _name_while(Statement<l, LetRemote<binding, body>>)
{
  return Statement<l, LetRemote<binding, DECT(name_while<seqnum, depth>(body{}))>>{};
}

template <char, char, typename l, typename Var, typename Expr>
constexpr auto _name_while(Statement<l, Assignment<Var, Expr>> a)
{
  return a;
}

template <char seqnum, char depth, typename l, typename cond, typename body>
constexpr auto _name_while(Statement<l, While<cond, body>>)
{
  return Statement<l, While<cond, DECT(name_while<seqnum, depth + 1>(body{})), 1, seqnum, depth>>{};
}

template <char seqnum, char depth, typename l, typename cond, typename then, typename els>
constexpr auto _name_while(Statement<l, If<cond, then, els>>)
{
  return Statement<l, If<cond, DECT(name_while<seqnum, depth>(then{})), DECT(name_while<seqnum, depth>(els{}))>>{};
}

template <char, char>
constexpr auto _name_while_stmt(Sequence<> a)
{
  return a;
}

template <char seqnum, char depth, typename stmt, typename... stmts>
constexpr auto _name_while_stmt(Sequence<stmt, stmts...>)
{
  return Sequence<DECT(name_while<seqnum, depth>(stmt{}))>::append(_name_while_stmt<seqnum + 1, depth>(Sequence<stmts...>{}));
}

template <char seqnum, char depth, typename l, typename... stmts>
constexpr auto _name_while(Statement<l, Sequence<stmts...>>)
{
  return Statement<l, DECT(_name_while_stmt<seqnum + 1, depth>(Sequence<stmts...>{}))>{};
}

template <char seqnum, char depth, typename AST>
constexpr auto name_while(AST a)
{
  return _name_while<seqnum, depth>(a);
}

template <typename AST>
constexpr auto name_while_pass(AST a)
{
  return name_while<1, 1>(a);
}
}
}
}
