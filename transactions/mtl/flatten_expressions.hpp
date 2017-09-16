#pragma once
#include "mtl/AST_parse.hpp"
#include "mutils/CTString.hpp"
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
template <char seqnum, char depth, template <typename> class SubStatement, typename Expr>
constexpr auto remove_layer(Expression<Expr>);

template <char seqnum, char depth, template <typename> class SubStatement, typename S, typename F>
constexpr auto remove_layer(Expression<FieldPointerReference<S, F>>);

template <char seqnum, char depth, template <typename> class SubStatement, typename V>
constexpr auto remove_layer(Expression<Dereference<Expression<VarReference<V>>>>);

template <char seqnum, char depth, template <typename> class SubStatement, typename V>
constexpr auto remove_layer(Expression<IsValid<Expression<VarReference<V>>>>);
	
	template <char seqnum, char depth, template <typename> class SubStatement, typename l, typename V>
	constexpr auto remove_layer(Expression<Endorse<l,Expression<VarReference<V>>>>);

		template <char seqnum, char depth, template <typename> class SubStatement, typename l, typename V>
	constexpr auto remove_layer(Expression<Ensure<l,Expression<VarReference<V>>>>);
  
template <char seqnum, char depth, template <typename> class SubStatement, typename Name, typename Hndl, typename... args>
constexpr auto remove_layer(Expression<Operation<Name, Expression<VarReference<Hndl> >, operation_args_exprs<>, operation_args_varrefs<args...> > > a);

template <char seqnum, char depth, template <typename> class SubStatement, char op, typename L, typename R>
constexpr auto remove_layer(Expression<BinOp<op, Expression<VarReference<L>>, Expression<VarReference<R>>>>);

template <char seqnum, char depth, template <typename> class SubStatement, typename Obj, typename F>
constexpr auto remove_layer(Expression<FieldReference<Expression<VarReference<Obj>>, F>>)
{
  using new_name = generate_name<seqnum, depth>;
  return Statement<Let<Binding<new_name, Expression<FieldReference<Expression<VarReference<Obj> >, F> > >, SubStatement<Expression<VarReference<new_name> > > >  >{};
}

template <char seqnum, char depth, template <typename> class SubStatement, typename Name, typename Hndl, typename... args>
constexpr auto remove_layer(Expression<Operation<Name, Expression<VarReference<Hndl> >, operation_args_exprs<>, operation_args_varrefs<args...> > > a){
	using new_name = generate_name<seqnum, depth>;
	return Statement<Let<Binding<new_name, DECT(a)>,  SubStatement<Expression<VarReference<new_name> > > > >{};
}

template <char seqnum, char depth, template <typename> class SubStatement, typename Var>
constexpr auto remove_layer(Expression<VarReference<Var>>)
{
  static_assert(seqnum == 0 && seqnum == 1, "Error: cannot remove layer from this expression");
}

template <char seqnum, char depth, template <typename> class SubStatement, int i>
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

template <char seqnum, char depth, template <typename> class SubStatement, typename E>
struct remove_layer_str<seqnum, depth, SubStatement, Expression<Dereference<E>>>
{
  static_assert(!is_var_reference<E>::value);
  template <typename newE>
  using NewStatement = SubStatement<Expression<Dereference<newE>>>;
  using type = DECT(remove_layer<seqnum, depth, NewStatement>(E{}));
};

template <char seqnum, char depth, template <typename> class SubStatement, typename E>
struct remove_layer_str<seqnum, depth, SubStatement, Expression<IsValid<E>>>
{
  static_assert(!is_var_reference<E>::value);
  template <typename newE>
  using NewStatement = SubStatement<Expression<IsValid<newE>>>;
  using type = DECT(remove_layer<seqnum, depth, NewStatement>(E{}));
};

	template <char seqnum, char depth, template <typename> class SubStatement, typename l, typename E>
	struct remove_layer_str<seqnum, depth, SubStatement, Expression<Endorse<l,E>>>
{
  static_assert(!is_var_reference<E>::value);
  template <typename newE>
	  using NewStatement = SubStatement<Expression<Endorse<l,newE>>>;
  using type = DECT(remove_layer<seqnum, depth, NewStatement>(E{}));
};

	template <char seqnum, char depth, template <typename> class SubStatement, typename l, typename E>
	struct remove_layer_str<seqnum, depth, SubStatement, Expression<Ensure<l,E>>>
{
  static_assert(!is_var_reference<E>::value);
  template <typename newE>
	  using NewStatement = SubStatement<Expression<Ensure<l,newE>>>;
  using type = DECT(remove_layer<seqnum, depth, NewStatement>(E{}));
};

	
template <char seqnum, char depth, template <typename> class SubStatement, typename oper_name, typename var, typename expr_arg1, typename var_args, typename... rest_expr_args>
struct remove_layer_str<seqnum, depth, SubStatement, Expression<Operation<oper_name,Expression<VarReference<var> >,operation_args_exprs<Expression<VarReference<expr_arg1> >,rest_expr_args...>,var_args> > >
{
	//there is already a variable reference in the expr arguments list; move it over and try again
  static_assert(Expression<Operation<oper_name,Expression<VarReference<var> >,operation_args_exprs<Expression<VarReference<expr_arg1> >,rest_expr_args...>,var_args> >::is_valid::value,"");
	using type = DECT(remove_layer<seqnum,depth,SubStatement>(Expression<Operation<oper_name,Expression<VarReference<var> >,operation_args_exprs<rest_expr_args...>,DECT(var_args::append(Expression<VarReference<expr_arg1> >{}))> >{}));
};
  
  template <char seqnum, char depth, template <typename> class SubStatement, typename oper_name, typename var, typename expr_arg1, typename var_args, typename... rest_expr_args>
  struct remove_layer_str<seqnum, depth, SubStatement, Expression<Operation<oper_name,Expression<VarReference<var> >,operation_args_exprs<expr_arg1,rest_expr_args...>,var_args> > >
{
  static_assert(Expression<Operation<oper_name,Expression<VarReference<var> >,operation_args_exprs<expr_arg1,rest_expr_args...>,var_args> >::is_valid::value,"");
  template <typename new_expr>
  using NewStatement = SubStatement<Expression<Operation<oper_name, Expression<VarReference<var> >, operation_args_exprs<new_expr, rest_expr_args...>, var_args> > >;
	using type = DECT(remove_layer<seqnum,depth,NewStatement>(expr_arg1{}));
};
  
  template <char seqnum, char depth, template <typename> class SubStatement, typename oper_name, typename expr, typename expr_args, typename var_args>
  struct remove_layer_str<seqnum, depth, SubStatement, Expression<Operation<oper_name,expr,expr_args,var_args> > >
{
  static_assert(Expression<Operation<oper_name,expr,expr_args,var_args> >::is_valid::value,"");
  static_assert(!is_var_reference<expr>::value);
  template <typename new_expr>
  using NewStatement = SubStatement<Expression<Operation<oper_name, new_expr, expr_args,var_args> > >;
  using type = DECT(remove_layer<seqnum, depth, NewStatement>(expr{}));
};

template <char seqnum, char depth, template <typename> class SubStatement, typename V>
constexpr auto remove_layer(Expression<Dereference<Expression<VarReference<V>>>>)
{
  using new_name = mutils::String<'r', 'e', 'm', 'o', 't', 'e', '_', 'b', 'o', 'u', 'n', 'd', '_', 't', 'm', 'p', 0, seqnum, depth>;
  return Statement<LetRemote<Binding<new_name, Expression<VarReference<V>>>, SubStatement<Expression<VarReference<new_name>>>>>{};
}

template <char seqnum, char depth, template <typename> class SubStatement, typename V>
constexpr auto remove_layer(Expression<IsValid<Expression<VarReference<V>>>> a)
{
	using new_name = generate_name<seqnum,depth>;
	return Statement<Let<Binding<new_name, DECT(a)>, SubStatement<Expression<VarReference<new_name>>>>>{};
}

	template <char seqnum, char depth, template <typename> class SubStatement, typename l, typename V>
	constexpr auto remove_layer(Expression<Endorse<l,Expression<VarReference<V>>>> a)
{
	using new_name = generate_name<seqnum,depth>;
	return Statement<Let<Binding<new_name, DECT(a)>, SubStatement<Expression<VarReference<new_name>>>>>{};
}

		template <char seqnum, char depth, template <typename> class SubStatement, typename l, typename V>
	constexpr auto remove_layer(Expression<Ensure<l,Expression<VarReference<V>>>> a)
{
	using new_name = generate_name<seqnum,depth>;
	return Statement<Let<Binding<new_name, DECT(a)>, SubStatement<Expression<VarReference<new_name>>>>>{};
}

template <char seqnum, char depth, template <typename> class SubStatement, typename Expr>
constexpr auto remove_layer(Expression<Expr>)
{
  using partial = typename remove_layer_str<seqnum, depth, SubStatement, Expression<Expr>>::type;
  return partial{};
}

template <char seqnum, char depth, template <typename> class SubStatement, char op, typename L, typename R>
constexpr auto remove_layer(Expression<BinOp<op, Expression<VarReference<L>>, Expression<VarReference<R>>>> a)
{
  using new_name = generate_name<seqnum, depth>;
  return Statement<Let<Binding<new_name, DECT(a) >,
                       SubStatement<Expression<VarReference<new_name>>>>>{};
}

template <char seqnum, char depth, template <typename> class SubStatement, typename S, typename F>
constexpr auto remove_layer(Expression<FieldPointerReference<S, F>>)
{
  return remove_layer<seqnum, depth, SubStatement>(Expression<FieldReference<Expression<Dereference<S>>, F>>{});
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

template <char seqnum, char depth, typename name, int i, typename body>
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
                template <char seqnum, char depth, typename name, char op, int varL, typename varR, typename body>
                constexpr auto _flatten_exprs(Statement<Let<Binding<name,Expression<BinOp<op,Expression<Constant<varL>>,Expression<VarReference<varR>>>>>,
   body>>) {
                //already flat, moving on
                        return Statement<Let<Binding<name,Expression<BinOp<op,Expression<Constant<varL> >,Expression<VarReference<varR> > >> >,
                                                                                                 DECT(flatten_exprs<seqnum,depth+1>(body{}))>>{};
        }

                template <char seqnum, char depth, typename name, char op, typename varL, int varR, typename body>
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

template <char seqnum, char depth, typename name, int var, typename Field, typename body>
constexpr auto _flatten_exprs(Statement<Let<Binding<name, Expression<FieldReference<Expression<Constant<var>>, Field>>>, body>>)
{
  // already flat, moving on
  return Statement<Let<Binding<name, Expression<FieldReference<Expression<Constant<var>>, Field>>>, DECT(flatten_exprs<seqnum, depth + 1>(body{}))>>{};
}

template <char seqnum, char depth, typename name, typename h, typename body>
constexpr auto _flatten_exprs(Statement<Let<Binding<name, Expression<IsValid<h>>>, body>>)
{
  // already flat, moving on
  return Statement<Let<Binding<name, Expression<IsValid<h>>>, DECT(flatten_exprs<seqnum, depth + 1>(body{}))>>{};
}

	template <char seqnum, char depth, typename name, typename l, typename h, typename body>
	constexpr auto _flatten_exprs(Statement<Let<Binding<name, Expression<Endorse<l,h>>>, body>>)
{
  // already flat, moving on
	return Statement<Let<Binding<name, Expression<Endorse<l,h>>>, DECT(flatten_exprs<seqnum, depth + 1>(body{}))>>{};
}

		template <char seqnum, char depth, typename name, typename l, typename h, typename body>
	constexpr auto _flatten_exprs(Statement<Let<Binding<name, Expression<Ensure<l,h>>>, body>>)
{
  // already flat, moving on
	return Statement<Let<Binding<name, Expression<Ensure<l,h>>>, DECT(flatten_exprs<seqnum, depth + 1>(body{}))>>{};
}

	
template <char seqnum, char depth, typename name, typename oper_name, typename Hndl, typename var_args, typename body>
constexpr auto _flatten_exprs(Statement<Let<Binding<name, Expression<Operation<oper_name, Expression<VarReference<Hndl> >, operation_args_exprs<>, var_args>>>, body>>)
{
  // already flat, moving on
  return Statement<Let<Binding<name, Expression<Operation<oper_name, Expression<VarReference<Hndl> >, operation_args_exprs<>, var_args>>>,
					   DECT(flatten_exprs<seqnum, depth + 1>(body{}))>>{};
}

template <char seqnum, char depth, typename oper_name, typename Hndl, typename var_args>
constexpr auto _flatten_exprs(Statement<Operation<oper_name, Expression<VarReference<Hndl> >, operation_args_exprs<>, var_args> > a)
{
	//flat; we're done!
	return a;
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

template <char seqnum, char depth, typename expr>
constexpr auto _flatten_exprs(Statement<Return<Expression<VarReference<expr>>>>)
{
  // flat, move on
  return Statement<Return<Expression<VarReference<expr>>>>{};
}

template <char seqnum, char depth, typename var, typename expr>
struct flatten_exprs_str<seqnum, depth, Statement<Assignment<var, expr>>>
{
  static_assert(!is_var_reference<expr>::value);
  template <typename new_expr>
  using SubStatementR = Statement<Assignment<var, new_expr>>;
  using type = DECT(remove_layer<seqnum, depth, SubStatementR>(expr{}));
};

template <char seqnum, char depth, typename expr>
struct flatten_exprs_str<seqnum, depth, Statement<Return<expr>>>
{
  static_assert(!is_var_reference<expr>::value);
  template <typename new_expr>
  using SubStatementR = Statement<Return<new_expr>>;
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
constexpr auto
_flatten_exprs(Sequence<stmt, stmts...>)
{
  return Sequence<DECT(flatten_exprs<seqnum, depth>(stmt{}))>::append(DECT(flatten_exprs<seqnum + 1, depth>(Sequence<stmts...>{})){});
}

template <char seqnum, char depth, typename... stmts>
constexpr auto
_flatten_exprs(Statement<Sequence<stmts...>>)
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
constexpr auto _flatten_exprs(Statement<Let<Binding<name, expr>, body>>, std::enable_if_t<!is_var_reference<expr>::value>* = nullptr)
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

	template<typename n,typename e,typename binding, typename body>
	auto operation_to_statement_f(Statement<Let<Binding<n,e>, Statement<LetRemote<binding, body> > > >);

	template<typename n, typename name, typename hndl, typename v, typename... args>
	auto operation_to_statement_f(Statement<Let<Binding<n,Expression<Operation<name,hndl,args...> > >, Statement<Return<Expression<VarReference<v> > > > > >){
		return Statement<Operation<name,hndl,args...> >{};
	}

	template<typename n,typename e,typename binding, typename body>
	auto operation_to_statement_f(Statement<Let<Binding<n,e>, Statement<Let<binding, body> > > >){
		return Statement<Let<Binding<n,e>, DECT(operation_to_statement_f(Statement<Let<binding, body> >{}))> >{};
	}

	template<typename n,typename e, typename binding, typename body>
	auto operation_to_statement_f(Statement<LetRemote<Binding<n,e>, Statement<Let<binding, body> > > >){
		return Statement<LetRemote<Binding<n,e>, DECT(operation_to_statement_f(Statement<Let<binding, body> >{}))> >{};
	}

	template<typename n,typename e,typename binding, typename body>
	auto operation_to_statement_f(Statement<Let<Binding<n,e>, Statement<LetRemote<binding, body> > > >){
		return Statement<Let<Binding<n,e>, DECT(operation_to_statement_f(Statement<LetRemote<binding, body> >{}))> >{};
	}

	template<typename n,typename e, typename binding, typename body>
	auto operation_to_statement_f(Statement<LetRemote<Binding<n,e>, Statement<LetRemote<binding, body> > > >){
		return Statement<LetRemote<Binding<n,e>, DECT(operation_to_statement_f(Statement<LetRemote<binding, body> >{}))> >{};
	}

	template<typename T>
	using operation_to_statement = DECT(operation_to_statement_f(T{}));

	template<typename t> using ret_stmt = Statement<Return<t> >;
	
template <char seqnum, char depth, typename oper_name, typename Hndl, typename var_args, typename e1, typename... exprs>
constexpr auto _flatten_exprs(Statement<Operation<oper_name, Hndl, operation_args_exprs<e1,exprs...>, var_args> >)
{
	using operation = Operation<oper_name, Hndl, operation_args_exprs<e1,exprs...>, var_args>;
	constexpr auto ret_partial = flatten_exprs<seqnum+1,depth>(remove_layer<seqnum,depth,ret_stmt>(Expression<operation>{}));
	return operation_to_statement_f(ret_partial);
}

template <char seqnum, char depth, typename oper_name, typename Hndl, typename var_args>
constexpr auto _flatten_exprs(Statement<Operation<oper_name, Hndl, operation_args_exprs<>, var_args> >, std::enable_if_t<!is_var_reference<Hndl>::value >* = nullptr)
{
	using operation = Operation<oper_name, Hndl, operation_args_exprs<>, var_args>;
	constexpr auto ret_parital = flatten_exprs<seqnum+1,depth>(remove_layer<seqnum,depth,ret_stmt>(Expression<operation>{}));
	return operation_to_statement_f(ret_parital);
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

template <char seqnum, char depth, typename cond>
constexpr auto _flatten_exprs(Statement<Return<cond>>)
{
  static_assert(!is_var_reference<cond>::value);
  return _flatten_exprs_helper<seqnum, depth>(Statement<Return<cond>>{});
}

template <char seqnum, char depth, typename AST>
constexpr auto
flatten_exprs(AST a, std::enable_if_t<(depth < depth_limit)>*)
{
  return _flatten_exprs<seqnum, depth>(a);
}

template <char seqnum, char depth, typename AST>
constexpr auto
flatten_exprs(AST a, std::enable_if_t<(depth >= depth_limit)>*)
{
  return a;
}

template <char, char, typename AST>
constexpr auto desugar_while(const AST);

template <char, char, typename v, typename e>
constexpr auto
_desugar_while(const Binding<v, e> b)
{
  return b;
}

template <char, char, typename e>
constexpr auto
_desugar_while(const Expression<e> a)
{
  return a;
}

template <char seqnum, char depth, typename b, typename body>
constexpr auto
_desugar_while(const Statement<Let<b, body>>)
{
  return Statement<Let<b, DECT(desugar_while<seqnum, depth + 1>(body{}))>>{};
}

template <char seqnum, char depth, typename b, typename body>
constexpr auto
_desugar_while(const Statement<LetRemote<b, body>>)
{
  return Statement<LetRemote<b, DECT(desugar_while<seqnum, depth + 1>(body{}))>>{};
}

template <char, char, typename L, typename R>
constexpr auto
_desugar_while(const Statement<Assignment<L, R>> a)
{
  return a;
}

template <char, char, typename name, typename hndl, typename eargs, typename vargs>
constexpr auto
_desugar_while(const Statement<Operation<name,hndl,eargs,vargs>> a)
{
  return a;
}

template <char, char, typename R>
constexpr auto
_desugar_while(const Statement<Return<R>> a)
{
  return a;
}

template <char seqnum, char depth, typename c, typename t, typename e>
constexpr auto
_desugar_while(const Statement<If<c, t, e>>)
{
  return Statement<If<c, DECT(desugar_while<seqnum, depth + 1>(t{})), DECT(desugar_while<seqnum + 1, depth + 1>(e{}))>>{};
}

template <char seqnum, char depth, typename c, typename t>
constexpr auto
_desugar_while(const Statement<While<c, t>>)
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
constexpr auto
_desugar_while(Sequence<> a)
{
  return a;
}

template <char seqnum, char depth, typename S1, typename... Seq>
constexpr auto
_desugar_while(Sequence<S1, Seq...>)
{
  return Sequence<DECT(desugar_while<seqnum, depth>(S1{}))>::append(desugar_while<seqnum + 1, depth>(Sequence<Seq...>{}));
}

template <char seqnum, char depth, typename... Seq>
constexpr auto
_desugar_while(Statement<Sequence<Seq...>>)
{
  return Statement<DECT(desugar_while<seqnum, depth>(Sequence<Seq...>{}))>{};
}

template <char s, char d, typename AST>
constexpr auto
desugar_while(const AST a)
{
  return _desugar_while<s, d>(a);
}
}
template <typename AST>
constexpr auto
flatten_expressions(AST a)
{
	/*
  using namespace mutils;
  using zeroname = String<'z', 'e', 'r', 'o', 0, 1>;
  using zeroval = Expression<Constant<0>>;
  using onename = String<'o', 'n', 'e', 0, 1>;
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
	//*/
	return anorm::flatten_exprs<1, 1>(anorm::desugar_while<1, 1>(a));
}
}
}
}
