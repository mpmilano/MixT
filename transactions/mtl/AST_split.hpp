#pragma once
#include "mutils/CTString.hpp"
#include <type_traits>
#include "mtl/mtlutils.hpp"
#include "mtl/split_phase_utils.hpp"
#include "mtl/label_utilities.hpp"
#include "mtl/AST_typecheck.hpp"

namespace myria {

template <typename>
struct Label;

namespace mtl {

namespace split_phase {

template <typename l1, typename l2>
constexpr bool are_equivalent(Label<l1>, Label<l2>)
{
  return (Label<l1>{} == labels::min_of(Label<l1>{}, Label<l2>{}))
    && (Label<l2>{} == labels::min_of(Label<l2>{}, Label<l1>{}));
}

// label is now per entire AST, rather than per node.

template <typename label>
struct AST;

template <typename l, typename label2, typename Yields, typename var, typename expr, typename phase_api>
constexpr auto let_remote_binding(phase_api, typecheck_phase::Binding<label2, Yields, var, expr>);

template <typename l>
struct AST<Label<l>>
{

  using label = Label<l>;

  template <typename>
  struct Statement;
  template <typename label, typename Yields, typename Var, typename Expr>
  struct Binding;

  template <typename Yields, typename>
  struct Expression;

  template <typename Struct, typename Field>
  struct FieldReference;
  template <typename subyield, typename Struct, char... Field>
  struct FieldReference<Expression<subyield, Struct>, mutils::String<Field...>>
  {
    using subexpr = FieldReference;
    using strexpr = typename Expression<subyield, Struct>::subexpr;
  };
  template <typename Yields, typename Struct, typename Field>
  struct Expression<Yields, FieldReference<Struct, Field>>
  {
    using yield = Yields;
    using subexpr = typename FieldReference<Struct, Field>::subexpr;

	template<template <typename, typename> class F, class Accum >
		using default_traverse = Expression<Yields, FieldReference<F<Accum,Struct>,Field > >;
	
	template<template <typename, typename> class F, class Accum, template<typename,typename> class Combine, class Default>
	  using default_recurse = F<Accum,Struct>;
	
	template<template <typename, typename> class F, class Accum >
		using fold = F<Accum,Struct>;
  };

  template <typename Var>
  struct VarReference;
  template <char... var>
  struct VarReference<mutils::String<var...>>
  {
    using subexpr = VarReference;
  };
  template <typename Yields, typename Var>
  struct Expression<Yields, VarReference<Var>>
  {
    using yield = Yields;
    using subexpr = typename VarReference<Var>::subexpr;
	template<template <typename, typename> class , class  >
		using default_traverse = Expression;
	
	template<template <typename, typename> class , class , template<typename,typename> class , class Default>
		using default_recurse = Default;

	template<template <typename, typename> class , class  Accum>
		using fold = Accum;
  };

  template <int>
  struct Constant
  {
  };
  template <int i>
  struct Expression<int, Constant<i>>
  {
    using yield = int;
    using subexpr = Constant<i>;

	template<template <typename, typename> class , class  >
		using default_traverse = Expression;
	
	template<template <typename, typename> class , class , template<typename,typename> class , class Default>
		using default_recurse = Default;
	
	template<template <typename, typename> class , class  Accum>
		using fold = Accum;
  };

  template <char op, typename L, typename R>
  struct BinOp;
  template <typename L, typename R, typename Yieldsl, typename Yieldsr>
  struct BinOp<'+', Expression<Yieldsl, L>, Expression<Yieldsr, R>>
  {
	  using yield = DECT(std::declval<Yieldsl>() + std::declval<Yieldsr>());
    using lexpr = typename Expression<Yieldsl, L>::subexpr;
    using rexpr = typename Expression<Yieldsr, R>::subexpr;
    using subexpr = BinOp;
  };
  template <typename L, typename R, typename Yieldsl, typename Yieldsr>
  struct BinOp<'*', Expression<Yieldsl, L>, Expression<Yieldsr, R>>
  {
	  using yield = DECT(std::declval<Yieldsl>() * std::declval<Yieldsr>());
    using lexpr = typename Expression<Yieldsl, L>::subexpr;
    using rexpr = typename Expression<Yieldsr, R>::subexpr;
    using subexpr = BinOp;
  };
  template <typename L, typename R, typename Yieldsl, typename Yieldsr>
  struct BinOp<'-', Expression<Yieldsl, L>, Expression<Yieldsr, R>>
  {
	  using yield = DECT(std::declval<Yieldsl>() - std::declval<Yieldsr>());
    using lexpr = typename Expression<Yieldsl, L>::subexpr;
    using rexpr = typename Expression<Yieldsr, R>::subexpr;
    using subexpr = BinOp;
  };
  template <typename L, typename R, typename Yieldsl, typename Yieldsr>
  struct BinOp<'/', Expression<Yieldsl, L>, Expression<Yieldsr, R>>
  {
	  using yield = DECT(std::declval<Yieldsl>() / std::declval<Yieldsr>());
    using lexpr = typename Expression<Yieldsl, L>::subexpr;
    using rexpr = typename Expression<Yieldsr, R>::subexpr;
    using subexpr = BinOp;
  };
  template <typename L, typename R, typename Yieldsl, typename Yieldsr>
  struct BinOp<'&', Expression<Yieldsl, L>, Expression<Yieldsr, R>>
  {
	  using yield = DECT(std::declval<Yieldsl>() && std::declval<Yieldsr>());
    using lexpr = typename Expression<Yieldsl, L>::subexpr;
    using rexpr = typename Expression<Yieldsr, R>::subexpr;
    using subexpr = BinOp;
  };
  template <typename L, typename R, typename Yieldsl, typename Yieldsr>
  struct BinOp<'|', Expression<Yieldsl, L>, Expression<Yieldsr, R>>
  {
	  using yield = DECT(std::declval<Yieldsl>() || std::declval<Yieldsr>());
    using lexpr = typename Expression<Yieldsl, L>::subexpr;
    using rexpr = typename Expression<Yieldsr, R>::subexpr;
    using subexpr = BinOp;
  };
  template <typename L, typename R, typename Yieldsl, typename Yieldsr>
  struct BinOp<'>', Expression<Yieldsl, L>, Expression<Yieldsr, R>>
  {
	  using yield = DECT(std::declval<Yieldsl>() > std::declval<Yieldsr>());
    using lexpr = typename Expression<Yieldsl, L>::subexpr;
    using rexpr = typename Expression<Yieldsr, R>::subexpr;
    using subexpr = BinOp;
  };
  template <typename L, typename R, typename Yieldsl, typename Yieldsr>
  struct BinOp<'<', Expression<Yieldsl, L>, Expression<Yieldsr, R>>
  {
	  using yield = DECT(std::declval<Yieldsl>() < std::declval<Yieldsr>());
    using lexpr = typename Expression<Yieldsl, L>::subexpr;
    using rexpr = typename Expression<Yieldsr, R>::subexpr;
    using subexpr = BinOp;
  };
  template <typename L, typename R, typename Yieldsl, typename Yieldsr>
  struct BinOp<'=', Expression<Yieldsl, L>, Expression<Yieldsr, R>>
  {
	  using yield = DECT(std::declval<Yieldsl>() == std::declval<Yieldsr>());
    using lexpr = typename Expression<Yieldsl, L>::subexpr;
    using rexpr = typename Expression<Yieldsr, R>::subexpr;
    using subexpr = BinOp;
  };
  template <typename Yields, char op, typename L, typename R>
  struct Expression<Yields, BinOp<op, L, R>>
  {
    using yield = Yields;
	static_assert(std::is_same<yield, typename BinOp<op, L, R>::yield >::value);
    using subexpr = typename BinOp<op, L, R>::subexpr;
	
	template<template <typename, typename> class F, class Accum >
		using default_traverse = Expression<typename BinOp<op,F<Accum,L>,F<Accum,R> >::yield,
											BinOp<op,F<Accum,L>,F<Accum,R> > >;
	
	template<template <typename, typename> class F, class Accum, template<typename,typename> class Combine, class Default>
		using default_recurse = Combine<F<Accum,L>, F<Accum,R> >;
	
	template<template <typename, typename> class F, class Accum >
		using fold = F<F<Accum,L>, R>;
  };

  template <void*...>
  struct GenerateTombstone
  {
    using subexpr = GenerateTombstone;
  };
  template <void*... useless>
  struct Expression<tracker::Tombstone, GenerateTombstone<useless...>>
  {
    using yield = tracker::Tombstone;
    using subexpr = typename GenerateTombstone<useless...>::subexpr;

	template<template <typename, typename> class F, class Accum >
		using default_traverse = Expression;
	
	template<template <typename, typename> class F, class Accum, template<typename,typename> class Combine, class Default>
	using default_recurse = Default;

	template<template <typename, typename> class F, class Accum >
	using fold = Accum;
  };

  template <typename Binding, typename Body>
  struct Let;
  template <typename label, typename Name, typename Expr, typename Body, typename Yields>
  struct Let<Binding<label, Yields, Name, Expr>, Statement<Body>>
  {
    using subcond = typename Binding<label, Yields, Name, Expr>::subexpr;
    using subbod = typename Statement<Body>::substatement;
    using substatement = Let;
  };
  template <typename Binding, typename Body>
  struct Statement<Let<Binding, Body>>
  {
    using substatement = typename Let<Binding, Body>::substatement;
	
	template<template <typename, typename> class F, class Accum >
		using default_traverse = Statement<Let<F<Accum,Binding >, F<Accum,Body> > >;
	
	template<template <typename, typename> class F, class Accum, template<typename,typename> class Combine, class Default>
		using default_recurse = Combine<F<Accum,Binding>, F<Accum,Body> >;
	
	template<template <typename, typename> class F, class Accum >
		using fold = F<F<Accum,Binding>, Body >;

  };

  template <typename Binding, typename Body>
  struct LetRemote;
  template <typename label, typename yield, typename Name, typename Expr, typename Body>
  struct LetRemote<Binding<label, yield, Name, Expr>, Statement<Body>>
  {
    using subcond = typename Binding<label, yield, Name, Expr>::subexpr;
    using subbod = typename Statement<Body>::substatement;
    using substatement = LetRemote;
  };
  template <typename Binding, typename Body>
  struct Statement<LetRemote<Binding, Body>>
  {
    using substatement = typename LetRemote<Binding, Body>::substatement;

	template<template <typename, typename> class F, class Accum >
		using default_traverse = Statement<LetRemote<F<Accum,Binding >, F<Accum,Body> > >;
	
	template<template <typename, typename> class F, class Accum, template<typename,typename> class Combine, class Default>
		using default_recurse = Combine<F<Accum,Binding>, F<Accum,Body> >;
	
	template<template <typename, typename> class F, class Accum >
		using fold = F<F<Accum,Binding>, Body >;
  };

	template <typename Hndl>
  struct IsValid;
	template <typename Hndl_y, typename Hndl_e>
		struct IsValid<Expression<Hndl_y, Hndl_e> >
  {
	  using subcond = Hndl_e;
	  using subexpr = IsValid;
	  using yield = bool;
  };
  template <typename Hndl>
	  struct Expression<bool, IsValid<Hndl>>
  {
	  using subexpr = typename IsValid<Hndl>::subexpr;
	  using yield = bool;

	  template<template <typename, typename> class F, class Accum >
	  using default_traverse = Expression<bool,IsValid<F<Accum,Hndl> > >;

  template<template <typename, typename> class F, class Accum, template<typename,typename> class Combine, class Default>
	  using default_recurse = F<Accum, Hndl>;

  template<template <typename, typename> class F, class Accum >
	  using fold = F<Accum,Hndl>;
  };

  template <typename oper_name, typename Hndl, typename... args>
	struct Operation
  {
    using subcond = typename Hndl::subexpr;
    using substatement = Operation;
	  using subexpr = Operation;
  };
  template <typename oper_name, typename Hndl, typename... args>
	  struct Statement<Operation<oper_name,Hndl,args...> >
  {
	  using substatement = typename Operation<oper_name,Hndl, args...>::substatement;
	  using subexpr = substatement;

	template<template <typename, typename> class F, class Accum >
	using default_traverse = Statement<Operation<oper_name,F<Accum,Hndl >, F<Accum,args>... > >;
	
	  template<template <typename, typename> class F, class Accum, template<typename,typename> class Combine, class Default>
	  using default_recurse = Combine_all<Combine,Default,F<Accum, Hndl>, F<Accum,args>...>;
	  
	  template<template <typename, typename> class F, class Accum >
	  using fold = Fold_all<F,Accum, Hndl,args...>;
	  
  };
  template <typename y, typename oper_name, typename Hndl, typename... args>
	  struct Expression<y,Operation<oper_name,Hndl,args...> >
  {
	  using substatement = typename Operation<oper_name,Hndl, args...>::substatement;
	  using subexpr = substatement;
	  using yield = y;

	  template<template <typename, typename> class F, class Accum >
	  using default_traverse = Expression<yield,
										Operation<oper_name,F<Accum,Hndl >, F<Accum,args>... > >;
	  
	  template<template <typename, typename> class F, class Accum, template<typename,typename> class Combine, class Default>
	  using default_recurse = Combine_all<Combine,Default,F<Accum, Hndl>, F<Accum,args>...>;
	  
	  template<template <typename, typename> class F, class Accum >
	  using fold = Fold_all<F,Accum, Hndl,args...>;
  };

  template <typename Var, typename Expr>
  struct Assignment;
  template <typename Expr, typename yield, typename var>
  struct Assignment<Expression<yield, var>, Expression<yield, Expr>>
  {
    using subl = typename Expression<yield, var>::subexpr;
    using subr = typename Expression<yield, Expr>::subexpr;
    using substatement = Assignment;
  };
  template <typename Var, typename Expr>
  struct Statement<Assignment<Var, Expr>>
  {
    using substatement = typename Assignment<Var, Expr>::substatement;

	template<template <typename, typename> class F, class Accum >
		using default_traverse = Statement<Assignment<F<Accum,Var >, F<Accum,Expr> > >;
	
	template<template <typename, typename> class F, class Accum, template<typename,typename> class Combine, class Default>
		using default_recurse = Combine<F<Accum,Var>, F<Accum,Expr> >;
	
	template<template <typename, typename> class F, class Accum >
		using fold = F<F<Accum,Var>, Expr >;
  };
  template <typename Expr>
  struct Return;
  template <typename y, typename Expr>
  struct Return<Expression<y, Expr>>
  {
    using substatement = Return;
    using subexpr = typename Expression<y, Expr>::subexpr;
  };
  template <typename Expr>
  struct Statement<Return<Expr>>
  {
    using substatement = typename Return<Expr>::substatement;
	
	template<template <typename, typename> class F, class Accum >
		using default_traverse = Statement<Return<F<Accum,Expr> > >;
  
	template<template <typename, typename> class F, class Accum, template<typename,typename> class Combine, class Default>
		using default_recurse = F<Accum,Expr>;
	
	template<template <typename, typename> class F, class Accum >
		using fold = F<Accum,Expr>;
  };

  template <typename>
  struct WriteTombstone;
  template <char... str>
    struct WriteTombstone<Expression<tracker::Tombstone,VarReference<mutils::String<str...> > > >
  {
    using substatement = WriteTombstone;
  };

  template <typename T>
  struct Statement<WriteTombstone<T>>
  {
    using substatement = typename WriteTombstone<T>::substatement;

	template<template <typename, typename> class F, class Accum >
		using default_traverse = Statement<WriteTombstone<F<Accum,T> > >;
	
	template<template <typename, typename> class F, class Accum, template<typename,typename> class Combine, class Default>
		using default_recurse = F<Accum,T>;
	
	template<template <typename, typename> class F, class Accum >
		using fold = F<Accum,T>;
  };

  template <typename>
  struct AccompanyWrite;
  template <typename e, typename y>
  struct AccompanyWrite<Expression<y, e>>
  {
    using substatement = AccompanyWrite;
    using subexpr = typename Expression<y, e>::subexpr;
  };
  template <typename T>
  struct Statement<AccompanyWrite<T>>
  {
    using substatement = typename AccompanyWrite<T>::substatement;

	template<template <typename, typename> class F, class Accum >
		using default_traverse = Statement<AccompanyWrite<F<Accum,T> > >;
	
	template<template <typename, typename> class F, class Accum, template<typename,typename> class Combine, class Default>
		using default_recurse = F<Accum,T>;
	
	template<template <typename, typename> class F, class Accum >
		using fold = F<Accum,T>;
  };

  template <typename Var>
  struct IncrementOccurance;
  template <char... var>
  struct IncrementOccurance<mutils::String<var...>>
  {
    using substatement = IncrementOccurance;
  };

  template <typename Var>
  struct IncrementRemoteOccurance;
  template <char... var>
  struct IncrementRemoteOccurance<mutils::String<var...>>
  {
    using substatement = IncrementRemoteOccurance;
  };

  template <typename label, typename type, typename var, typename... ops>
  struct IncrementOccurance<Expression<Handle<label,type,ops...>, VarReference<var> > >
  {
	  using subhandle = Handle<label,type,ops...>;
	  using handle_type = typename subhandle::type;
    using substatement = IncrementOccurance;
  };
  template <typename Var>
  struct Statement<IncrementOccurance<Var>>
  {
    using substatement = typename IncrementOccurance<Var>::substatement;
  };
  template <typename Var>
  struct Statement<IncrementRemoteOccurance<Var>>
  {
    using substatement = typename IncrementRemoteOccurance<Var>::substatement;
  };

  template <typename Var> struct RefreshRemoteOccurance;
  
  template <typename label, typename type, typename var, typename... ops>
  struct RefreshRemoteOccurance<Expression<Handle<label,type,ops...>, VarReference<var> > >
  {
	  using subhandle = Handle<label,type,ops...>;
	  using handle_type = typename subhandle::type;
    using substatement = RefreshRemoteOccurance;
  };
  template <typename Var>
  struct Statement<RefreshRemoteOccurance<Var>>
  {
    using substatement = typename RefreshRemoteOccurance<Var>::substatement;
  };

  template <typename condition, typename then, typename els>
  struct If;
  template <typename condition, typename then, typename els>
  struct If<Expression<bool, condition>, Statement<then>, Statement<els>>
  {
    using subcond = typename Expression<bool, condition>::subexpr;
    using subthen = typename Statement<then>::substatement;
    using subelse = typename Statement<els>::substatement;
    using substatement = If;
  };
  template <typename condition, typename then, typename els>
  struct Statement<If<condition, then, els>>
  {
	  using substatement = typename If<condition, then, els>::substatement;
  
	  template<template <typename, typename> class F, class Accum >
		  using default_traverse = Statement<If<F<Accum,condition>, F<Accum,then>, F<Accum,els> > >;

	  template<template <typename, typename> class F, class Accum, template<typename,typename> class Combine, class Default>
		  using default_recurse = Combine< Combine<F<Accum,condition>, F<Accum,then> >, F<Accum,els> >;

	  template<template <typename, typename> class F, class Accum >
		  using fold = F<F<F<Accum,condition>, then >, els>;
  };

  template <typename condition, typename body, char...>
  struct While;
  template <typename condition, typename body, char... name>
  struct While<Expression<bool, condition>, Statement<body>, 1, name...>
  {
    using subcond = typename Expression<bool, condition>::subexpr;
    using subthen = typename Statement<body>::substatement;
    using substatement = While;
  };
  template <typename condition, typename Body, char... name>
  struct Statement<While<condition, Body, 1, name...>>
  {
    using substatement = typename While<condition, Body, 1, name...>::substatement;

	template<template <typename, typename> class F, class Accum >
	  using default_traverse = Statement<While<F<Accum,condition>, F<Accum,Body> > >;

	template<template <typename, typename> class F, class Accum, template<typename,typename> class Combine, class Default>
		using default_recurse = Combine<F<Accum,condition>, F<Accum,Body> >;
	
	template<template <typename, typename> class F, class Accum >
		using fold = F<F<Accum,condition>, Body >;
  };

  template <typename body, char...>
  struct ForEach;
  template <typename body, char... name>
  struct ForEach<Statement<body>, 1, name...>
  {
    using subbody = typename Statement<body>::substatement;
    using substatement = ForEach;
  };
  template <typename Body, char... name>
  struct Statement<ForEach<Body, name...>>
  {
    using substatement = typename ForEach<Body, name...>::substatement;
  };

  template <typename>
  struct is_statement;

  template <typename... Statements>
  struct Sequence
  {
    using substatements = mutils::typelist<typename Statements::substatement...>;
    using substatement = Sequence;
    static_assert(mutils::forall<is_statement<Statements>::value...>(), "Error: Sequence is not of statements!");

    template <typename... more>
    static constexpr Sequence<Statements..., more...> append(Sequence<more...>)
    {
      return Sequence<Statements..., more...>{};
    }

    template <typename stmt>
    static constexpr Sequence<Statements..., Statement<stmt>> append(Statement<stmt>)
    {
      return Sequence<Statements..., Statement<stmt>>{};
    }
  };
  template <typename b1, typename... Body>
  struct Statement<Sequence<b1, Body...>>
  {
    using substatement = typename Sequence<b1, Body...>::substatement;
    using fst_stmt = b1;

	template<template <typename, typename> class F, class Accum >
		using default_traverse = Statement<Sequence<F<Accum,Body>...> >;
	
	template<template <typename, typename> class F, class Accum, template<typename,typename> class Combine, class Default>
		using default_recurse = Combine_all<Combine,Default,F<Accum,Body>... >;
	
	template<template <typename, typename> class F, class Accum >
		using fold = Fold_all<F,Accum,Body...>;
  };

  template <typename... Body>
  struct Statement<Sequence<Body...>>
  {
    static_assert(sizeof...(Body) == 0);
    using fst_stmt = Statement<Sequence<>>;
    using substatement = typename Sequence<Body...>::substatement;
  };

  template <typename label, typename Yield, typename Expr, char... name>
  struct Binding<Label<label>, Yield, mutils::String<name...>, Expr>
  {
    using subexpr = Binding;
    using subsubexpr = typename Expr::subexpr;
	  using expr = Expr;
	  using var = mutils::String<name...>;

	  template<template <typename, typename> class F, class Accum >
	  using default_traverse = Binding<Label<label>, Yield, var, F<Accum,expr> >;

  template<template <typename, typename> class F, class Accum, template<typename,typename> class Combine, class Default>
	  using default_recurse = F<Accum,expr>;

  template<template <typename, typename> class F, class Accum >
	  using fold = F<Accum,expr>;
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

  template <typename>
  struct is_expression;

  template <typename yield, typename stmt>
  struct is_expression<Expression<yield, stmt>> : public std::true_type
  {
  };

  template <typename>
  struct is_expression : public std::false_type
  {
  };

  template <typename>
  struct is_ast_node;

  template <typename stmt>
  struct is_ast_node<Statement<stmt>> : public std::true_type
  {
  };

  template <typename yield, typename expr>
  struct is_ast_node<Expression<yield, expr>> : public std::true_type
  {
  };

  template <typename>
  struct is_ast_node : public std::false_type
  {
  };

  template <typename T>
  constexpr bool is_ast_node_f(const T&)
  {
    return is_ast_node<T>::value;
  }

  // traversal functions; implemented in split_phase.hpp
  template <typename label2, typename Yields, typename var, typename expr, typename phase_api>
  static constexpr auto _collect_phase(phase_api, typecheck_phase::Binding<label2, Yields, var, expr>);

  template <typename Yields, typename label2, typename Str, typename phase_api>
		static constexpr auto _collect_phase(phase_api, typecheck_phase::Expression<label2, Yields, typecheck_phase::VarReference<Str>>,
																				 std::enable_if_t<!builtins::is_builtin<Str>::value>* = nullptr);

	  template <typename Yields, typename label2, typename Str, typename phase_api>
		static constexpr auto _collect_phase(phase_api, typecheck_phase::Expression<label2, Yields, typecheck_phase::VarReference<Str>>,
																				 std::enable_if_t<builtins::is_builtin<Str>::value>* = nullptr);

  template <typename _binding, typename Body, typename phase_api>
  static constexpr auto _collect_phase(phase_api, typecheck_phase::Statement<label, typecheck_phase::Let<_binding, Body>>);

  template <typename label2, typename Binding, typename Body, typename phase_api>
  static constexpr auto _collect_phase(phase_api, typecheck_phase::Statement<label2, typecheck_phase::Let<Binding, Body>>,
                                       std::enable_if_t<!are_equivalent(label{}, label2{})> const* const = nullptr);

  template <typename _binding, typename Body, typename old_api>
	  static constexpr auto _collect_phase(old_api, typecheck_phase::Statement<label, typecheck_phase::LetRemote<_binding, Body>>);

  template <typename label2, typename _binding, typename Body, typename phase_api>
  static constexpr auto _collect_phase(phase_api, typecheck_phase::Statement<label2, typecheck_phase::LetRemote<_binding, Body>>,
                                       std::enable_if_t<!are_equivalent(Label<l>{}, label2{})> const* const = nullptr);

  template <typename oper_name, typename hndl, typename old_api, typename... args>
	  static constexpr auto _collect_phase(old_api, typecheck_phase::Statement<label, typecheck_phase::Operation<oper_name,hndl, args...>>,
																				 std::enable_if_t<!builtins::is_builtin<oper_name>::value>* = nullptr);
  
  template <typename label2, typename oper_name, typename hndl, typename phase_api, typename... args>
	  static constexpr auto _collect_phase(phase_api, typecheck_phase::Statement<label2, typecheck_phase::Operation<oper_name,hndl, args...>>,
																				 std::enable_if_t<!are_equivalent(Label<l>{}, label2{}) > const* const = nullptr);

	template <typename oper_name, typename hndl, typename old_api, typename... args>
		static constexpr auto _collect_phase(old_api, typecheck_phase::Statement<label, typecheck_phase::Operation<oper_name,hndl, args...>>,
																				 std::enable_if_t<builtins::is_builtin<oper_name>::value>* = nullptr);

  template <typename Var, typename Expr, typename phase_api>
  static constexpr auto _collect_phase(phase_api, typecheck_phase::Statement<label, typecheck_phase::Assignment<Var, Expr>>);

  template <typename Var, typename Expr, typename label2, typename phase_api>
  static constexpr auto _collect_phase(phase_api, typecheck_phase::Statement<label2, typecheck_phase::Assignment<Var, Expr>>,
                                       std::enable_if_t<!are_equivalent(Label<l>{}, label2{})> const* const = nullptr);

  template <typename Expr, typename phase_api>
  static constexpr auto _collect_phase(phase_api, typecheck_phase::Statement<label, typecheck_phase::Return<Expr>>);

  template <typename Expr, typename label2, typename phase_api>
  static constexpr auto _collect_phase(phase_api, typecheck_phase::Statement<label2, typecheck_phase::Return<Expr>>,
                                       std::enable_if_t<!are_equivalent(Label<l>{}, label2{})> const* const = nullptr);

  template <typename phase_api, typename T>
    static constexpr auto _collect_phase(phase_api, typecheck_phase::Statement<label, typecheck_phase::WriteTombstone<T> >);

  template <typename label2, typename phase_api, typename T>
    static constexpr auto _collect_phase(phase_api, typecheck_phase::Statement<label2, typecheck_phase::WriteTombstone<T> >,
                                       std::enable_if_t<!are_equivalent(Label<l>{}, label2{})> const* const = nullptr);

  template <typename Expr, typename phase_api>
  static constexpr auto _collect_phase(phase_api, typecheck_phase::Statement<label, typecheck_phase::AccompanyWrite<Expr>>);

  template <typename Expr, typename label2, typename phase_api>
  static constexpr auto _collect_phase(phase_api, typecheck_phase::Statement<label2, typecheck_phase::AccompanyWrite<Expr>>,
                                       std::enable_if_t<!are_equivalent(Label<l>{}, label2{})> const* const = nullptr);

  template <typename cond, typename then, typename old_api, char... name>
  static constexpr auto _collect_phase(old_api, typecheck_phase::Statement<Label<l>, typecheck_phase::While<cond, then, name...>>);

  template <typename label2, typename cond, typename then, typename old_api, char... name>
  static constexpr auto _collect_phase(old_api, typecheck_phase::Statement<label2, typecheck_phase::While<cond, then, name...>>,
                                       std::enable_if_t<!are_equivalent(Label<l>{}, label2{}) && !label2::flows_to(label{})> const* const = nullptr);

  template <typename label2, typename cond, typename then, typename phase_api, char... name>
  static constexpr auto _collect_phase(phase_api, typecheck_phase::Statement<label2, typecheck_phase::While<cond, then, name...>>,
                                       std::enable_if_t<!are_equivalent(Label<l>{}, label2{}) && label2::flows_to(label{})> const* const = nullptr);

  template <typename Yields, typename label2, typename Str, typename Fld, typename phase_api>
  static constexpr auto _collect_phase(phase_api, typecheck_phase::Expression<label2, Yields, typecheck_phase::FieldReference<Str, Fld>>);

  template <typename y, typename label2, typename oper_name, typename hndl, typename old_api, typename... args>
	  static constexpr auto _collect_phase(old_api, typecheck_phase::Expression<label2, y,typecheck_phase::Operation<oper_name,hndl, args...> >);
  
  template <typename y, typename label2, typename hndl, typename old_api>
	  static constexpr auto _collect_phase(old_api, typecheck_phase::Expression<label2,y, typecheck_phase::IsValid<hndl>>);

  template <int i, typename phase_api>
  static constexpr auto _collect_phase(phase_api, typecheck_phase::Expression<Label<top>, int, typecheck_phase::Constant<i>>);

  template <char op, typename label2, typename Yields, typename L, typename R, typename phase_api>
  static constexpr auto _collect_phase(phase_api, typecheck_phase::Expression<label2, Yields, typecheck_phase::BinOp<op, L, R>>);

  template <typename phase_api>
  static constexpr auto _collect_phase(phase_api, typecheck_phase::Expression<Label<top>, tracker::Tombstone, typecheck_phase::GenerateTombstone>);

  template <typename cond, typename label2, typename _then, typename _els, typename phase_api>
  static constexpr auto _collect_phase(phase_api, typecheck_phase::Statement<label2, typecheck_phase::If<cond, _then, _els>>,
                                       std::enable_if_t<label2::flows_to(label{})> const* const = nullptr);

  template <typename cond, typename label2, typename _then, typename _els, typename phase_api>
  static constexpr auto _collect_phase(phase_api, typecheck_phase::Statement<label2, typecheck_phase::If<cond, _then, _els>>,
                                       std::enable_if_t<!label2::flows_to(label{})> const* const = nullptr);

  template <typename label2, typename phase_api, typename... seq>
  static constexpr auto _collect_phase(phase_api, typecheck_phase::Statement<label2, typecheck_phase::Sequence<seq...>>);

  template <typename _AST, typename phase_api>
	  static constexpr auto collect_phase(phase_api, _AST);

  template <typename>
  struct is_ast;

  template <typename stmt>
  struct is_ast<Statement<stmt>> : public std::true_type
  {
  };

  template <typename yields, typename expr>
  struct is_ast<Expression<yields, expr>> : public std::true_type
  {
  };

  template <typename lvl, typename yields, typename var, typename expr>
  struct is_ast<Binding<lvl, yields, var, expr>> : public std::true_type
  {
  };

  template <typename>
  struct is_ast : public std::false_type
  {
  };

  // collapse sequences
  template <typename>
  struct count_sequence;
  template <typename... seq>
  struct count_sequence<Statement<Sequence<seq...>>> : public std::integral_constant<std::size_t, (0 + ... + count_sequence<seq>::value)>
  {
  };
  template <typename stmt>
  struct count_sequence<Statement<stmt>> : public std::true_type
  {
  };

  template <typename T>
  using is_empty_sequence = std::integral_constant<bool, count_sequence<T>::value == 0>;

  static constexpr auto collapse1(Sequence<> a) { return a; }

  template <typename b, typename e>
  static constexpr auto collapse1(Statement<Let<b, e>> a)
  {
    return a;
  }

  template <typename b, typename e>
  static constexpr auto collapse1(Statement<LetRemote<b, e>> a)
  {
    return a;
  }

  template <typename y, typename h>
  static constexpr auto collapse1(Expression<y,IsValid<h>> a)
  {
    return a;
  }

  template <typename n, typename h, typename... a>
  static constexpr auto collapse1(Statement<Operation<n, h, a...>> _a)
  {
    return _a;
  }

  template <typename y, typename n, typename h, typename... a>
  static constexpr auto collapse1(Expression<y,Operation<n, h, a...>> _a)
  {
    return _a;
  }

  template <typename c, typename t, typename e>
  static constexpr auto collapse1(Statement<If<c, t, e>> a)
  {
    return a;
  }

  template <typename c, typename t>
  static constexpr auto collapse1(Statement<Assignment<c, t>> a)
  {
    return a;
  }

  template <typename T>
  static constexpr auto collapse1(Statement<Return<T>> a)
  {
    return a;
  }

  template<typename T>
  static constexpr auto collapse1(Statement<WriteTombstone<T>> a) { return a; }

  template <typename T>
  static constexpr auto collapse1(Statement<AccompanyWrite<T>> a)
  {
    return a;
  }

  template <typename c, typename e, char... name>
  static constexpr auto collapse1(Statement<While<c, e, name...>> a)
  {
    return a;
  }

  template <typename e, char... name>
  static constexpr auto collapse1(Statement<ForEach<e, name...>> a)
  {
    return a;
  }

  template <typename name>
  static constexpr auto collapse1(Statement<IncrementOccurance<name>> a)
  {
    return a;
  }

  template <typename name>
  static constexpr auto collapse1(Statement<IncrementRemoteOccurance<name>> a)
  {
    return a;
  }


  template <typename name>
  static constexpr auto collapse1(Statement<RefreshRemoteOccurance<name>> a)
  {
    return a;
  }

  template <typename... seq>
  static constexpr auto collapse1(Statement<Sequence<seq...>>)
  {
    return AST::collapse1(Sequence<seq...>{});
  }

  template <typename s1>
  static constexpr auto collapse1(Sequence<s1>, std::enable_if_t<!is_empty_sequence<s1>::value>* = nullptr)
  {
    return AST::collapse1(s1{});
  }

  template <typename s1, typename... seq>
  static constexpr auto collapse1(Sequence<s1, seq...>, std::enable_if_t<is_empty_sequence<s1>::value>* = nullptr)
  {
    return AST::collapse1(Sequence<seq...>{});
  }

  template <typename s1, typename s2, typename... seq>
  static constexpr auto collapse1(Sequence<s1, s2, seq...>, std::enable_if_t<!is_empty_sequence<s1>::value>* = nullptr)
  {
    return Sequence<>::append(AST::collapse1(s1{})).append(AST::collapse1(Sequence<s2, seq...>{}));
  }

  template <typename... seq>
  static constexpr auto collapse(Statement<Sequence<seq...>>)
  {
    constexpr auto ret = AST::collapse1(Sequence<>::append(AST::collapse1(Sequence<seq...>{})));
    return Statement<DECT(Sequence<>::append(ret))>{};
  }
};

template <typename label, typename stmt>
constexpr auto extract_label(typename AST<label>::template Statement<stmt>)
{
  return label{};
}

template <typename label, typename yields, typename expr>
constexpr auto extract_label(typename AST<label>::template Expression<yields, expr>)
{
  return label{};
}

template <typename label, typename yields, typename var, typename expr>
constexpr auto extract_label(typename AST<label>::template Binding<yields, var, expr>)
{
  return label{};
}
}
}
}
