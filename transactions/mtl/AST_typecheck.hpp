#pragma once

#include "CTString.hpp"
#include <type_traits>
#include <vector>
#include "mtlutils.hpp"
#include "top.hpp"
#include "environments.hpp"

namespace myria {
namespace mtl {

// Note! For this and all future phases, expressions can only be one level
// deep before we hit VarReference, and Let is the only Statement that doesn't
// immediately hit VarReference.  We are now encoding this constraint at the
// AST level.
namespace typecheck_phase {

template <typename label, typename>
struct Statement;
template <typename l, typename Yields, typename Var, typename Expr>
struct Binding;

template <typename l, typename Yields, typename>
struct Expression;

template <typename Var>
struct VarReference;
template <char... var>
struct VarReference<mutils::String<var...>>
{
  using subexpr = VarReference;
};
template <typename l, typename Yields, typename Var>
struct Expression<Label<l>, Yields, VarReference<Var>>
{
  using label = Label<l>;
  using yield = Yields;
  using subexpr = typename VarReference<Var>::subexpr;
};

template <typename Struct, typename Field>
struct FieldReference;
template <typename sublabel, typename subyield, typename structname, char... Field>
struct FieldReference<Expression<sublabel, subyield, VarReference<structname>>, mutils::String<Field...>>
{
  using subexpr = FieldReference;
};
template <typename l, typename Yields, typename Struct, typename Field>
struct Expression<Label<l>, Yields, FieldReference<Struct, Field>>
{
  using label = Label<l>;
  using yield = Yields;
  using subexpr = typename FieldReference<Struct, Field>::subexpr;
};

template <long long>
struct Constant
{
};
template <long long i>
struct Expression<Label<top>, long long, Constant<i>>
{
  using label = Label<top>;
  using yield = long long;
  using subexpr = Constant<i>;
};

template <char op, typename L, typename R>
struct BinOp;
template <typename L, typename R, typename ll, typename Yieldsl, typename lr, typename Yieldsr>
struct BinOp<'+', Expression<Label<ll>, Yieldsl, VarReference<L>>, Expression<Label<lr>, Yieldsr, VarReference<R>>>
{
  using subexpr = BinOp;
  using yield = DECT(std::declval<Yieldsl>() + std::declval<Yieldsr>());
  using lexpr = typename VarReference<L>::subexpr;
  using rexpr = typename VarReference<R>::subexpr;
};
template <typename L, typename R, typename ll, typename Yieldsl, typename lr, typename Yieldsr>
struct BinOp<'*', Expression<Label<ll>, Yieldsl, VarReference<L>>, Expression<Label<lr>, Yieldsr, VarReference<R>>>
{
  using subexpr = BinOp;
  using yield = DECT(std::declval<Yieldsl>() * std::declval<Yieldsr>());
  using lexpr = typename VarReference<L>::subexpr;
  using rexpr = typename VarReference<R>::subexpr;
};
template <typename L, typename R, typename ll, typename Yieldsl, typename lr, typename Yieldsr>
struct BinOp<'-', Expression<Label<ll>, Yieldsl, VarReference<L>>, Expression<Label<lr>, Yieldsr, VarReference<R>>>
{
  using subexpr = BinOp;
  using yield = DECT(std::declval<Yieldsl>() - std::declval<Yieldsr>());
  using lexpr = typename VarReference<L>::subexpr;
  using rexpr = typename VarReference<R>::subexpr;
};
template <typename L, typename R, typename ll, typename Yieldsl, typename lr, typename Yieldsr>
struct BinOp<'/', Expression<Label<ll>, Yieldsl, VarReference<L>>, Expression<Label<lr>, Yieldsr, VarReference<R>>>
{
  using subexpr = BinOp;
  using yield = DECT(std::declval<Yieldsl>() / std::declval<Yieldsr>());
  using lexpr = typename VarReference<L>::subexpr;
  using rexpr = typename VarReference<R>::subexpr;
};
template <typename L, typename R, typename ll, typename Yieldsl, typename lr, typename Yieldsr>
struct BinOp<'&', Expression<Label<ll>, Yieldsl, VarReference<L>>, Expression<Label<lr>, Yieldsr, VarReference<R>>>
{
  using subexpr = BinOp;
  using yield = DECT(std::declval<Yieldsl>() && std::declval<Yieldsr>());
  using lexpr = typename VarReference<L>::subexpr;
  using rexpr = typename VarReference<R>::subexpr;
};
template <typename L, typename R, typename ll, typename Yieldsl, typename lr, typename Yieldsr>
struct BinOp<'|', Expression<Label<ll>, Yieldsl, VarReference<L>>, Expression<Label<lr>, Yieldsr, VarReference<R>>>
{
  using subexpr = BinOp;
  using yield = DECT(std::declval<Yieldsl>() || std::declval<Yieldsr>());
  using lexpr = typename VarReference<L>::subexpr;
  using rexpr = typename VarReference<R>::subexpr;
};
template <typename L, typename R, typename ll, typename Yieldsl, typename lr, typename Yieldsr>
struct BinOp<'>', Expression<Label<ll>, Yieldsl, VarReference<L>>, Expression<Label<lr>, Yieldsr, VarReference<R>>>
{
  using subexpr = BinOp;
  using yield = DECT(std::declval<Yieldsl>() > std::declval<Yieldsr>());
  using lexpr = typename VarReference<L>::subexpr;
  using rexpr = typename VarReference<R>::subexpr;
};
template <typename L, typename R, typename ll, typename Yieldsl, typename lr, typename Yieldsr>
struct BinOp<'<', Expression<Label<ll>, Yieldsl, VarReference<L>>, Expression<Label<lr>, Yieldsr, VarReference<R>>>
{
  using subexpr = BinOp;
  using yield = DECT(std::declval<Yieldsl>() < std::declval<Yieldsr>());
  using lexpr = typename VarReference<L>::subexpr;
  using rexpr = typename VarReference<R>::subexpr;
};
template <typename L, typename R, typename ll, typename Yieldsl, typename lr, typename Yieldsr>
struct BinOp<'=', Expression<Label<ll>, Yieldsl, VarReference<L>>, Expression<Label<lr>, Yieldsr, VarReference<R>>>
{
  using subexpr = BinOp;
  using yield = DECT(std::declval<Yieldsl>() == std::declval<Yieldsr>());
  using lexpr = typename VarReference<L>::subexpr;
  using rexpr = typename VarReference<R>::subexpr;
};
template <typename l, char op, typename L, typename R>
struct Expression<Label<l>, typename BinOp<op, L, R>::yield, BinOp<op, L, R>>
{
  using label = Label<l>;
  using yield = typename BinOp<op, L, R>::yield;
  using subexpr = typename BinOp<op, L, R>::subexpr;
};

template <typename Binding, typename Body>
struct Let;
template <typename Name, typename Expr, typename Body, typename bl, typename Yields, typename sl>
struct Let<Binding<bl, Yields, Name, Expr>, Statement<sl, Body>>
{
  using substatement = Let;
  using subexpr = typename Expr::subexpr;
  using subbody = typename Body::substatement;
  using var = Name;
};
template <typename l, typename Binding, typename Body>
struct Statement<Label<l>, Let<Binding, Body>>
{
  using label = Label<l>;
  using substatement = typename Let<Binding, Body>::substatement;
};

template <typename Binding, typename Body>
struct LetRemote;
template <typename l, typename yield, typename Name, typename Exprl, typename Expry, typename ExprN, typename bodyl, typename Body>
struct LetRemote<Binding<l, yield, Name, Expression<Exprl, Expry, VarReference<ExprN>>>, Statement<bodyl, Body>>
{
  using substatement = LetRemote;
  using subbody = typename Body::substatement;
  using subexpr = typename Expression<Exprl, Expry, VarReference<ExprN>>::subexpr;
};

template <typename l, typename bl, typename y, typename var, typename exprl, typename expry, typename expr, typename Body>
struct Statement<Label<l>, LetRemote<Binding<bl, y, var, Expression<exprl, expry, expr>>, Body>>
{
  using label = Label<l>;
  // expry should be a handle.  Could maybe enforce that.
  using handle_label = typename expry::label;
  using expr_label = exprl;
  using substatement = typename LetRemote<Binding<bl, y, var, Expression<exprl, expry, expr>>, Body>::substatement;
};

template <typename Var, typename Expr>
struct Assignment;
template <typename Expr, typename l, typename yield, typename varl, typename var>
struct Assignment<Expression<varl, yield, var>, Expression<l, yield, VarReference<Expr>>>
{
  using substatement = Assignment;
  using subexpr = typename Expression<l, yield, VarReference<Expr>>::subexpr;
  using subvar = typename Expression<varl, yield, var>::subexpr;
};
template <typename l, typename Var, typename Expr>
struct Statement<Label<l>, Assignment<Var, Expr>>
{
  using label = Label<l>;
  using substatement = typename Assignment<Var, Expr>::substatement;
};

template <typename condition, typename then, typename els>
struct If;
template <typename exprl, typename condition, typename labelthen, typename then, typename labelelse, typename els>
struct If<Expression<exprl, bool, VarReference<condition>>, Statement<labelthen, then>, Statement<labelelse, els>>
{
  using substatement = If;
  using subexpr = typename Expression<exprl, bool, VarReference<condition>>::subexpr;
  using subthen = typename Statement<labelthen, then>::substatement;
  using subelse = typename Statement<labelelse, els>::substatement;
};
template <typename l, typename condition, typename then, typename els>
struct Statement<Label<l>, If<condition, then, els>>
{
  using label = Label<l>;
  using substatement = typename If<condition, then, els>::substatement;
};

template <typename condition, typename body, char...>
struct While;
template <typename labelcond, typename condition, typename labelbody, typename body, char... name>
struct While<Expression<labelcond, bool, VarReference<condition>>, Statement<labelbody, body>, name...>
{
  using substatement = While;
  using subexpr = typename Expression<labelcond, bool, VarReference<condition>>::subexpr;
  using subbody = typename Statement<labelbody, body>::substatement;
};
template <typename l, typename condition, typename Body, char... name>
struct Statement<Label<l>, While<condition, Body, name...>>
{
  using label = Label<l>;
  using substatement = typename While<condition, Body, name...>::substatement;
};

template <typename>
struct is_statement;

template <typename... Statements>
struct Sequence
{

  static_assert(mutils::forall<is_statement<Statements>::value...>(), "Error: Sequence is not of statements!");

  using substatements = mutils::typelist<typename Statements::substatement...>;
  using substatement = Sequence;

  template <typename... more>
  static constexpr Sequence<Statements..., more...> append(Sequence<more...>)
  {
    return Sequence<Statements..., more...>{};
  }
};
template <typename l, typename... Body>
struct Statement<Label<l>, Sequence<Body...>>
{
  using label = Label<l>;
  using substatement = typename Sequence<Body...>::substatement;
};

template <typename l, typename Yield, typename EYield, typename Exprl, typename ExprN, char... name>
struct Binding<Label<l>, Yield, mutils::String<name...>, Expression<Exprl, EYield, ExprN>>
{
  using label = Label<l>;
  using subexpr = typename Expression<Exprl, EYield, ExprN>::subexpr;
  using var = mutils::String<name...>;
};

template <typename label, typename Stmt>
constexpr auto label_of(Statement<label, Stmt>)
{
  return label{};
}

template <typename label, typename yield, typename Stmt>
constexpr auto label_of(Expression<label, yield, Stmt>)
{
  return label{};
}

template <typename>
struct is_statement;

template <typename l, typename stmt>
struct is_statement<Statement<l, stmt>> : public std::true_type
{
};

template <typename>
struct is_statement : public std::false_type
{
};

template <typename>
struct is_expression;

template <typename l, typename yield, typename stmt>
struct is_expression<Expression<l, yield, stmt>> : public std::true_type
{
};

template <typename>
struct is_expression : public std::false_type
{
};

template <typename>
struct is_ast_node;

template <typename lbl, typename stmt>
struct is_ast_node<Statement<lbl, stmt>> : public std::true_type
{
};

template <typename lbl, typename yield, typename expr>
struct is_ast_node<Expression<lbl, yield, expr>> : public std::true_type
{
};

template <typename>
struct is_ast_node : public std::false_type
{
};

template <typename T>
constexpr bool
is_ast_node_f(const T&)
{
  return is_ast_node<T>::value;
}
}
}
}
