#pragma once

#include "mtl/AST_typecheck.hpp"
#include "mtl/mtlutils.hpp"

namespace myria {
namespace mtl {
namespace typecheck_phase {
	namespace easy_processing_phase{

		template<char, char, bool returns_allowed> struct accum{};

		
	
	template <typename accum, typename AST>
	constexpr auto easy_processing(accum, AST a);

		template<typename accum, typename AST>
		using Easy_Processing = DECT(easy_processing(accum{},AST{}));

// expressions
		template <typename accum, typename l, typename yields, typename expr>
constexpr auto _easy_processing(accum, Expression<l, yields, expr> a)
{
  return a;
}

// statements
		template <char seqnum, char depth, bool b, typename l, typename binding, typename body>
		constexpr auto _easy_processing(accum<seqnum,depth,b>, Statement<l, Let<binding, body>>)
{
  return Statement<l, Let<binding, Easy_Processing<accum<seqnum,depth,b>, body > > >{};
}

		template <char seqnum, char depth, bool b, typename l, typename binding, typename body>
constexpr auto _easy_processing(accum<seqnum,depth,b>, Statement<l, LetRemote<binding, body>>)
{
  return Statement<l, LetRemote<binding, Easy_Processing<accum<seqnum,depth,b>,body> > >{};
}

template <char seqnum, char depth, bool b, typename l, typename Expr>
constexpr auto _easy_processing(accum<seqnum,depth,b>, Statement<l, Return<Expr>> a)
{
	static_assert(b,"Error: Return only from top level");
  return a;
}

		template <char seqnum, char depth, bool b, typename l, typename cond, typename body>
constexpr auto _easy_processing(accum<seqnum,depth,b>, Statement<l, While<cond, body>>)
{
  return Statement<l, While<cond, Easy_Processing<accum<seqnum,depth+1,false>, body>, 1, seqnum, depth>>{};
}

		template <char seqnum, char depth, bool b, typename l, typename cond, typename then, typename els>
constexpr auto _easy_processing(accum<seqnum,depth,b>, Statement<l, If<cond, then, els>>)
{
  return Statement<l, If<cond, Easy_Processing<accum<seqnum,depth,false>, then>, Easy_Processing<accum<seqnum,depth,false>,els> > >{};
}

		template <typename accum>
constexpr auto _easy_processing_stmt(accum, Sequence<> a)
{
  return a;
}
		
		template <char seqnum, char depth, bool returns_allowed, typename l, typename stmt, typename... stmts>
		constexpr auto _easy_processing_stmt(accum<seqnum,depth,returns_allowed>, Sequence<Statement<l,Return<stmt> >, stmts...> a)
{
	static_assert(returns_allowed, "Error: Return only from top level");
	static_assert(sizeof...(stmts) == 0, "Error: Dead code detected");
	return a;
}

		template <char seqnum, char depth, bool b, typename stmt, typename... stmts>
constexpr auto _easy_processing_stmt(accum<seqnum,depth,b>, Sequence<stmt, stmts...>)
{
  return Sequence<DECT(easy_processing(accum<seqnum,depth,b>{},stmt{}))>::append(_easy_processing_stmt(accum<seqnum+1,depth,b>{},Sequence<stmts...>{}));
}

		template <char seqnum, char depth, bool b, typename l, typename... stmts>
		constexpr auto _easy_processing(accum<seqnum,depth,b>, Statement<l, Sequence<stmts...>>)
{
  return Statement<l, DECT(_easy_processing_stmt(accum<seqnum + 1, depth, b>{},Sequence<stmts...>{}))>{};
}

//base / else case
template <typename accum, typename l, typename expr>
constexpr auto _easy_processing(accum, Statement<l, expr> a)
{
  return typename DECT(a)::template default_traverse<Easy_Processing, accum>{};
}

template <typename accum, typename AST>
constexpr auto easy_processing(accum, AST a)
{
  return _easy_processing(accum{}, a);
}
	}

template <typename AST>
constexpr auto easy_processing_pass(AST a)
{
  return easy_processing_phase::easy_processing(easy_processing_phase::accum<1,1,true>{}, a);
}
}
}
}
