#pragma once
#include "CTString.hpp"
#include <type_traits>
#include "mtlutils.hpp"
#include "top.hpp"

namespace myria {
namespace mtl {

namespace parse_phase {

template <typename>
struct Statement;
template <typename Var, typename Expr>
struct Binding;

template <typename>
struct Expression;

template <typename Struct, typename Field>
struct FieldReference;
template <typename Struct, char... Field>
struct FieldReference<Expression<Struct>, mutils::String<Field...>>
{
  using subexpr = FieldReference;
};
template <typename Struct, typename Field>
struct Expression<FieldReference<Struct, Field>>
{
  using subexpr = typename FieldReference<Struct, Field>::subexpr;
};

  template <typename Struct, typename Field>
struct FieldPointerReference;
template <typename Struct, char... Field>
struct FieldPointerReference<Expression<Struct>, mutils::String<Field...>>
{
  using subexpr = FieldPointerReference;
};
template <typename Struct, typename Field>
struct Expression<FieldPointerReference<Struct, Field>>
{
  using subexpr = typename FieldPointerReference<Struct, Field>::subexpr;
};

  template <typename Struct>
struct Dereference;
template <typename Struct>
struct Dereference<Expression<Struct>>
{
  using subexpr = Dereference;
};
template <typename Struct>
struct Expression<Dereference<Struct>>
{
  using subexpr = typename Dereference<Struct>::subexpr;
};

template <typename Var>
struct VarReference;
template <char... var>
struct VarReference<mutils::String<var...>>
{
  using subexpr = VarReference;
};
template <typename Var>
struct Expression<VarReference<Var>>
{
  using subexpr = typename VarReference<Var>::subexpr;
};

template <typename>
struct is_var_reference;
template <typename var>
struct is_var_reference<VarReference<var>> : public std::true_type
{
};

template <typename>
struct is_var_reference;
template <typename var>
struct is_var_reference<Expression<VarReference<var>>> : public std::true_type
{
};

template <typename>
struct is_var_reference : public std::false_type
{
};

template <int>
struct Constant
{
  using subexpr = Constant;
};
template <int i>
struct Expression<Constant<i>>
{
  using subexpr = typename Constant<i>::subexpr;
};

template <char op, typename L, typename R>
struct BinOp;
template <typename L, typename R>
struct BinOp<'+', Expression<L>, Expression<R>>
{
  using subexpr = BinOp;
};
template <typename L, typename R>
struct BinOp<'*', Expression<L>, Expression<R>>
{
  using subexpr = BinOp;
};
template <typename L, typename R>
struct BinOp<'-', Expression<L>, Expression<R>>
{
  using subexpr = BinOp;
};
template <typename L, typename R>
struct BinOp<'/', Expression<L>, Expression<R>>
{
  using subexpr = BinOp;
};
template <typename L, typename R>
struct BinOp<'&', Expression<L>, Expression<R>>
{
  using subexpr = BinOp;
};
template <typename L, typename R>
struct BinOp<'|', Expression<L>, Expression<R>>
{
  using subexpr = BinOp;
};
template <typename L, typename R>
struct BinOp<'<', Expression<L>, Expression<R>>
{
  using subexpr = BinOp;
};
template <typename L, typename R>
struct BinOp<'>', Expression<L>, Expression<R>>
{
  using subexpr = BinOp;
};
template <typename L, typename R>
struct BinOp<'=', Expression<L>, Expression<R>>
{
  using subexpr = BinOp;
};
template <char op, typename L, typename R>
struct Expression<BinOp<op, L, R>>
{
  using subexpr = typename BinOp<op, L, R>::subexpr;
};

template <typename>
struct is_depth_1;
template <typename var>
struct is_depth_1<VarReference<var>> : public std::true_type
{
};
template <char op, typename varl, typename varr>
struct is_depth_1<BinOp<op, Expression<VarReference<varl>>, Expression<VarReference<varr>>>> : public std::true_type
{
};
template <int l>
struct is_depth_1<Constant<l>> : public std::true_type
{
};
template <typename var, typename field>
struct is_depth_1<FieldReference<VarReference<var>, field>> : public std::true_type
{
};

template <typename>
struct is_depth_1;
template <typename E>
struct is_depth_1<Expression<E>> : public is_depth_1<E>
{
};
template <typename>
struct is_depth_1 : public std::false_type
{
};

template <typename Binding, typename Body>
struct Let;
template <typename Name, typename Expr, typename Body>
struct Let<Binding<Name, Expr>, Statement<Body>>
{
};
template <typename Binding, typename Body>
struct Statement<Let<Binding, Body>>
{
};

template <typename Binding, typename Body>
struct LetRemote;
template <typename Name, typename Expr, typename Body>
struct LetRemote<Binding<Name, Expr>, Statement<Body>>
{
};
template <typename Binding, typename Body>
struct Statement<LetRemote<Binding, Body>>
{
};

template <typename Binding, typename Body, typename label>
struct LetEndorsed;
template <typename Name, typename Expr, typename Body, typename label>
struct LetEndorsed<Binding<Name, Expr>, Statement<Body>, Label<label> >
{
};
template <typename Binding, typename Body, typename label>
struct Statement<LetEndorsed<Binding, Body,label> >
{
};

template <typename Var, typename Expr>
struct Assignment;
template <typename Expr, typename Var>
struct Assignment<Expression<Var>, Expression<Expr>>
{
};
template <typename Var, typename Expr>
struct Statement<Assignment<Var, Expr>>
{
};

template <typename Expr>
struct Return;
template <typename Expr>
struct Return<Expression<Expr>>
{
};
template <typename Expr>
struct Statement<Return<Expr>>
{
};

template <typename condition, typename then, typename els>
struct If;
template <typename condition, typename then, typename els>
struct If<Expression<condition>, Statement<then>, Statement<els>>
{
};
template <typename condition, typename then, typename els>
struct Statement<If<condition, then, els>>
{
};

template <typename condition, typename body>
struct While;
template <typename condition, typename body>
struct While<Expression<condition>, Statement<body>>
{
};
template <typename condition, typename Body>
struct Statement<While<condition, Body>>
{
};

template <typename>
struct is_expression;

template <typename stmt>
struct is_expression<Expression<stmt>> : public std::true_type
{
};

template <typename>
struct is_expression : public std::false_type
{
};

template <typename>
struct is_statement;

template <typename stmt>
struct is_statement<Statement<stmt>> : public std::true_type
{
};

template <typename>
struct is_statement : public std::false_type
{
};

template <typename... Statements>
struct Sequence
{

  static_assert(mutils::forall<is_statement<Statements>::value...>(), "Error: Sequence is not of statements!");

  template <typename... more>
  static constexpr Sequence<Statements..., more...> append(Sequence<more...>)
  {
    return Sequence<Statements..., more...>{};
  }
};
template <typename... Body>
struct Statement<Sequence<Body...>>
{
};

template <typename Expr, char... name>
struct Binding<mutils::String<name...>, Expr>
{
};

template <typename>
struct is_ast_node;

template <typename stmt>
struct is_ast_node<Statement<stmt>> : public std::true_type
{
};

template <typename expr>
struct is_ast_node<Expression<expr>> : public std::true_type
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
