#pragma once
#include "CTString.hpp"
#include "AST_parse.hpp"
namespace myria {

template <typename>
struct Label;
namespace mtl {

constexpr std::size_t depth_limit = 1000;

namespace parse_phase {
namespace anorm {
template <char seqnum, char depth, typename AST>
constexpr auto flatten_exprs(AST a, std::enable_if_t<(depth < depth_limit)>* = nullptr);

template <char seqnum, char depth, typename AST>
constexpr auto flatten_exprs(AST, std::enable_if_t<(depth >= depth_limit)>* = nullptr);

template <char seqnum, char depth>
using generate_name = mutils::String<'a', 'n', 'o', 'r', 'm', seqnum, depth>;

// expressions
template <char seqnum, char depth, template <typename> class SubStatement, typename Obj, typename F>
constexpr auto remove_layer(Expression<FieldReference<Expression<VarReference<Obj>>, F>>)
{
  using new_name = generate_name<seqnum, depth>;
  return Statement<Let<Binding<new_name, Expression<FieldReference<Expression<VarReference<Obj>>, F>>>, SubStatement<Expression<VarReference<new_name>>>>>{};
}

template <char seqnum, char depth, template <typename> class SubStatement, typename Var>
constexpr auto remove_layer(Expression<VarReference<Var>>)
{
  static_assert(seqnum == 0 && seqnum == 1, "Error: cannot remove layer from this expression");
}

template <char seqnum, char depth, template <typename> class SubStatement, long long i>
constexpr auto remove_layer(Expression<Constant<i>>)
{
  using new_name = generate_name<seqnum, depth>;
  return Statement<Let<Binding<new_name, Expression<Constant<i>>>, SubStatement<Expression<VarReference<new_name>>>>>{};
}

template <char, char, template <typename> class, typename T>
struct remove_layer_str;

template <char seqnum, char depth, template <typename> class SubStatement, typename S, typename F>
struct remove_layer_str<seqnum, depth, SubStatement, Expression<FieldReference<S, F>>>
{
  static_assert(!is_var_reference<S>::value);
  template <typename newS>
  using NewStatement = SubStatement<Expression<FieldReference<newS, F>>>;
  using type = DECT(remove_layer<seqnum, depth, NewStatement>(S{}));
};

template <char seqnum, char depth, template <typename> class SubStatement, char op, typename L, typename R>
struct remove_layer_str<seqnum, depth, SubStatement, Expression<BinOp<op, L, Expression<VarReference<R>>>>>
{
  template <typename newL>
  using NewStatement = SubStatement<Expression<BinOp<op, newL, Expression<VarReference<R>>>>>;
  using type = DECT(remove_layer<seqnum, depth, NewStatement>(L{}));
};

template <char seqnum, char depth, template <typename> class SubStatement, char op, typename L, typename R>
struct remove_layer_str<seqnum, depth, SubStatement, Expression<BinOp<op, Expression<VarReference<L>>, R>>>
{
  template <typename newR>
  using NewStatement = SubStatement<Expression<BinOp<op, Expression<VarReference<L>>, newR>>>;
  using type = DECT(remove_layer<seqnum, depth, NewStatement>(R{}));
};

template <char seqnum, char depth, template <typename> class SubStatement, char op, typename L, typename R>
struct remove_layer_str<seqnum, depth, SubStatement, Expression<BinOp<op, L, R>>>
{
  static_assert(!is_var_reference<L>::value || !is_var_reference<R>::value);
  template <typename newL>
  using NewStatement = SubStatement<Expression<BinOp<op, newL, R>>>;
  using type = DECT(remove_layer<seqnum, depth, NewStatement>(L{}));
};

template <char seqnum, char depth, template <typename> class SubStatement, typename Expr>
constexpr auto remove_layer(Expression<Expr>)
{
  using partial = typename remove_layer_str<seqnum, depth, SubStatement, Expression<Expr>>::type;
  return partial{};
}

template <char seqnum, char depth, template <typename> class SubStatement, char op, typename L, typename R>
constexpr auto remove_layer(Expression<BinOp<op, Expression<VarReference<L>>, Expression<VarReference<R>>>>)
{
  using new_name = generate_name<seqnum, depth>;
  return Statement<Let<Binding<new_name, Expression<BinOp<op, Expression<VarReference<L>>, Expression<VarReference<R>>>>>,
                       SubStatement<Expression<VarReference<new_name>>>>>{};
}

template <char, char, typename>
struct flatten_exprs_str;

// statements
// let statements need to have any depth-1 expression
template <char seqnum, char depth, typename name, typename var, typename body>
constexpr auto _flatten_exprs(Statement<Let<Binding<name, Expression<VarReference<var>>>, body>>)
{
  // already flat, moving on
  return Statement<Let<Binding<name, Expression<VarReference<var>>>, DECT(flatten_exprs<seqnum, depth + 1>(body{}))>>{};
}

template <char seqnum, char depth, typename name, long long i, typename body>
constexpr auto _flatten_exprs(Statement<Let<Binding<name, Expression<Constant<i>>>, body>>)
{
  // already flat, moving on
  return Statement<Let<Binding<name, Expression<Constant<i>>>, DECT(flatten_exprs<seqnum, depth + 1>(body{}))>>{};
}

template <char seqnum, char depth, typename name, char op, typename varL, typename varR, typename body>
constexpr auto _flatten_exprs(Statement<Let<Binding<name, Expression<BinOp<op, Expression<VarReference<varL>>, Expression<VarReference<varR>>>>>, body>>)
{
  // already flat, moving on
  return Statement<Let<Binding<name, Expression<BinOp<op, Expression<VarReference<varL>>, Expression<VarReference<varR>>>>>,
                       DECT(flatten_exprs<seqnum, depth + 1>(body{}))>>{};
}
/*
                template <char seqnum, char depth, typename name, char op, long long varL, typename varR, typename body>
                constexpr auto _flatten_exprs(Statement<Let<Binding<name,Expression<BinOp<op,Expression<Constant<varL>>,Expression<VarReference<varR>>>>>,
   body>>) {
                //already flat, moving on
                        return Statement<Let<Binding<name,Expression<BinOp<op,Expression<Constant<varL> >,Expression<VarReference<varR> > >> >,
                                                                                                 DECT(flatten_exprs<seqnum,depth+1>(body{}))>>{};
        }

                template <char seqnum, char depth, typename name, char op, typename varL, long long varR, typename body>
                constexpr auto _flatten_exprs(Statement<Let<Binding<name,Expression<BinOp<op,Expression<VarReference<varL>>,Expression<Constant<varR>>>>>,
   body>>) {
                //already flat, moving on
                        return Statement<Let<Binding<name,Expression<BinOp<op,Expression<VarReference<varL> >,Expression<Constant<varR> > >> >,
                                                                                                 DECT(flatten_exprs<seqnum,depth+1>(body{}))>>{};
                }//*/

template <char seqnum, char depth, typename name, typename var, typename Field, typename body>
constexpr auto _flatten_exprs(Statement<Let<Binding<name, Expression<FieldReference<Expression<VarReference<var>>, Field>>>, body>>)
{
  // already flat, moving on
  return Statement<Let<Binding<name, Expression<FieldReference<Expression<VarReference<var>>, Field>>>, DECT(flatten_exprs<seqnum, depth + 1>(body{}))>>{};
}

template <char seqnum, char depth, typename name, long long var, typename Field, typename body>
constexpr auto _flatten_exprs(Statement<Let<Binding<name, Expression<FieldReference<Expression<Constant<var>>, Field>>>, body>>)
{
  // already flat, moving on
  return Statement<Let<Binding<name, Expression<FieldReference<Expression<Constant<var>>, Field>>>, DECT(flatten_exprs<seqnum, depth + 1>(body{}))>>{};
}

// other statements need to have only var references
template <char seqnum, char depth, typename name, typename expr, typename body>
struct flatten_exprs_str<seqnum, depth, Statement<Let<Binding<name, expr>, body>>>
{
  static_assert(!is_var_reference<expr>::value);
  static_assert(!is_depth_1<expr>::value);
  template <typename new_expr>
  using SubStatement = Statement<Let<Binding<name, new_expr>, body>>;
  using type = DECT(remove_layer<seqnum, depth, SubStatement>(expr{}));
};

template <char seqnum, char depth, typename name, typename var, typename body>
constexpr auto _flatten_exprs(Statement<LetRemote<Binding<name, Expression<VarReference<var>>>, body>>)
{
  // already flat, moving on
  return Statement<LetRemote<Binding<name, Expression<VarReference<var>>>, DECT(flatten_exprs<seqnum, depth + 1>(body{}))>>{};
}

template <char seqnum, char depth, typename name, typename expr, typename body>
struct flatten_exprs_str<seqnum, depth, Statement<LetRemote<Binding<name, expr>, body>>>
{
  static_assert(!is_var_reference<expr>::value);
  template <typename new_expr>
  using SubStatementRemote = Statement<LetRemote<Binding<name, new_expr>, body>>;
  using type = DECT(remove_layer<seqnum, depth, SubStatementRemote>(expr{}));
};

template <char seqnum, char depth, typename var, typename expr>
constexpr auto _flatten_exprs(Statement<Assignment<var, Expression<VarReference<expr>>>>)
{
  // flat, move on
  return Statement<Assignment<var, Expression<VarReference<expr>>>>{};
}

template <char seqnum, char depth, typename var, typename expr>
struct flatten_exprs_str<seqnum, depth, Statement<Assignment<var, expr>>>
{
  static_assert(!is_var_reference<expr>::value);
  template <typename new_expr>
  using SubStatementR = Statement<Assignment<var, new_expr>>;
  using type = DECT(remove_layer<seqnum, depth, SubStatementR>(expr{}));
};

template <char seqnum, char depth, typename cond, typename body>
constexpr auto _flatten_exprs(Statement<While<Expression<VarReference<cond>>, body>>)
{
  return Statement<While<Expression<VarReference<cond>>, DECT(flatten_exprs<seqnum, depth + 1>(body{}))>>{};
}

template <char seqnum, char depth, typename cond, typename then>
struct flatten_exprs_str<seqnum, depth, Statement<While<cond, then>>>
{
  static_assert(!is_var_reference<cond>::value);
  template <typename new_cond>
  using SubStatement = Statement<While<new_cond, then>>;
  using type = DECT(remove_layer<seqnum, depth, SubStatement>(cond{}));
};

template <char seqnum, char depth, typename cond, typename then, typename els>
constexpr auto _flatten_exprs(Statement<If<Expression<VarReference<cond>>, then, els>>)
{
  return Statement<If<Expression<VarReference<cond>>, DECT(flatten_exprs<seqnum, depth + 1>(then{})), DECT(flatten_exprs<seqnum + 1, depth + 1>(els{}))>>{};
}

template <char seqnum, char depth, typename cond, typename then, typename els>
struct flatten_exprs_str<seqnum, depth, Statement<If<cond, then, els>>>
{
  static_assert(!is_var_reference<cond>::value);
  template <typename new_cond>
  using SubStatement = Statement<If<new_cond, then, els>>;
  using type = DECT(remove_layer<seqnum, depth, SubStatement>(cond{}));
};

template <char, char>
constexpr auto _flatten_exprs(Sequence<>)
{
  return Sequence<>{};
}
template <char seqnum, char depth, typename stmt, typename... stmts>
constexpr auto _flatten_exprs(Sequence<stmt, stmts...>)
{
  return Sequence<DECT(flatten_exprs<seqnum, depth>(stmt{}))>::append(DECT(flatten_exprs<seqnum + 1, depth>(Sequence<stmts...>{})){});
}

template <char seqnum, char depth, typename... stmts>
constexpr auto _flatten_exprs(Statement<Sequence<stmts...>>)
{
  return Statement<DECT(flatten_exprs<seqnum, depth>(Sequence<stmts...>{}))>{};
}

template <char seqnum, char depth, typename stmt>
constexpr auto _flatten_exprs_helper(Statement<stmt>)
{
  using partial = typename flatten_exprs_str<seqnum, depth, Statement<stmt>>::type;
  return DECT(flatten_exprs<seqnum, depth + 1>(partial{})){};
}

template <char seqnum, char depth, typename name, typename expr, typename body>
constexpr auto _flatten_exprs(Statement<Let<Binding<name, expr>, body>>)
{
  static_assert(!is_var_reference<expr>::value);
  // static_assert(print_obj<Let<Binding<name,expr>, body>>(),"Error: Let fell into default case");
  return _flatten_exprs_helper<seqnum, depth>(Statement<Let<Binding<name, expr>, body>>{});
}

template <char seqnum, char depth, typename name, typename expr, typename body>
constexpr auto _flatten_exprs(Statement<LetRemote<Binding<name, expr>, body>>)
{
  static_assert(!is_var_reference<expr>::value);
  return _flatten_exprs_helper<seqnum, depth>(Statement<LetRemote<Binding<name, expr>, body>>{});
}

template <char seqnum, char depth, typename var, typename expr>
constexpr auto _flatten_exprs(Statement<Assignment<var, expr>>)
{
  static_assert(!is_var_reference<var>::value || !is_var_reference<expr>::value);
  return _flatten_exprs_helper<seqnum, depth>(Statement<Assignment<var, expr>>{});
}

template <char seqnum, char depth, typename cond, typename then>
constexpr auto _flatten_exprs(Statement<While<cond, then>>)
{
  static_assert(!is_var_reference<cond>::value);
  return _flatten_exprs_helper<seqnum, depth>(Statement<While<cond, then>>{});
}

template <char seqnum, char depth, typename cond, typename then, typename els>
constexpr auto _flatten_exprs(Statement<If<cond, then, els>>)
{
  static_assert(!is_var_reference<cond>::value);
  return _flatten_exprs_helper<seqnum, depth>(Statement<If<cond, then, els>>{});
}

template <char seqnum, char depth, typename AST>
constexpr auto flatten_exprs(AST a, std::enable_if_t<(depth < depth_limit)>*)
{
  return _flatten_exprs<seqnum, depth>(a);
}

template <char seqnum, char depth, typename AST>
constexpr auto flatten_exprs(AST a, std::enable_if_t<(depth >= depth_limit)>*)
{
  return a;
}

template <char, char, typename AST>
constexpr auto desugar_while(const AST);

template <char, char, typename v, typename e>
constexpr auto _desugar_while(const Binding<v, e> b)
{
  return b;
}

template <char, char, typename e>
constexpr auto _desugar_while(const Expression<e> a)
{
  return a;
}

template <char seqnum, char depth, typename b, typename body>
constexpr auto _desugar_while(const Statement<Let<b, body>> a)
{
  return Statement<Let<b, DECT(desugar_while<seqnum, depth + 1>(body{}))>>{};
}

template <char seqnum, char depth, typename b, typename body>
constexpr auto _desugar_while(const Statement<LetRemote<b, body>> a)
{
  return Statement<LetRemote<b, DECT(desugar_while<seqnum, depth + 1>(body{}))>>{};
}

template <char, char, typename L, typename R>
constexpr auto _desugar_while(const Statement<Assignment<L, R>> a)
{
  return a;
}

template <char seqnum, char depth, typename c, typename t, typename e>
constexpr auto _desugar_while(const Statement<If<c, t, e>>)
{
  return Statement<If<c, DECT(desugar_while<seqnum, depth + 1>(t{})), DECT(desugar_while<seqnum + 1, depth + 1>(e{}))>>{};
}

template <char seqnum, char depth, typename c, typename t>
constexpr auto _desugar_while(const Statement<While<c, t>>)
{
  using namespace mutils;
  using condition_name = String<'w', 'h', 'i', 'l', 'e', seqnum, depth>;
  using trueval = Expression<VarReference<String<'t', 'r', 'u', 'e'>>>;
  using falseval = Expression<VarReference<String<'f', 'a', 'l', 's', 'e'>>>;
  return Statement<Let<
    Binding<condition_name, trueval>,
    Statement<While<Expression<VarReference<condition_name>>, Statement<If<c, DECT(desugar_while<seqnum, depth + 1>(t{})),
                                                                           Statement<Assignment<Expression<VarReference<condition_name>>, falseval>>>>>>>>{};
}

template <char, char>
constexpr auto _desugar_while(Sequence<> a)
{
  return a;
}

template <char seqnum, char depth, typename S1, typename... Seq>
constexpr auto _desugar_while(Sequence<S1, Seq...>)
{
  return Sequence<DECT(desugar_while<seqnum, depth>(S1{}))>::append(desugar_while<seqnum + 1, depth>(Sequence<Seq...>{}));
}

template <char seqnum, char depth, typename... Seq>
constexpr auto _desugar_while(Statement<Sequence<Seq...>>)
{
  return Statement<DECT(desugar_while<seqnum, depth>(Sequence<Seq...>{}))>{};
}

template <char s, char d, typename AST>
constexpr auto desugar_while(const AST a)
{
  return _desugar_while<s, d>(a);
}
}
template <typename AST>
constexpr auto flatten_expressions(AST a)
{
  using namespace mutils;
  using zeroname = String<'z', 'e', 'r', 'o', 1>;
  using zeroval = Expression<Constant<0>>;
  using onename = String<'o', 'n', 'e', 1>;
  using oneval = Expression<Constant<1>>;
  using truename = String<'t', 'r', 'u', 'e'>;
  using falsename = String<'f', 'a', 'l', 's', 'e'>;
  using trueval = Expression<BinOp<'=', Expression<VarReference<zeroname>>, Expression<VarReference<zeroname>>>>;
  using falseval = Expression<BinOp<'=', Expression<VarReference<zeroname>>, Expression<VarReference<onename>>>>;
  return Statement<
    Let<Binding<zeroname, zeroval>,
        Statement<Let<Binding<onename, oneval>,
                      Statement<Let<Binding<truename, trueval>,
                                    Statement<Let<Binding<falsename, falseval>, DECT(anorm::flatten_exprs<1, 1>(anorm::desugar_while<1, 1>(a)))>>>>>>>>{};
}
}
}
}
