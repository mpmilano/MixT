#pragma once

#include "AST_split.hpp"
#include "runnable_transaction.hpp"

namespace myria {
namespace mtl {
namespace split_phase {

	template<typename newl, typename l, typename e>
	constexpr auto __relabel(e);
	
	template<typename newl, typename l, typename e>
	using relabel = DECT(__relabel<newl,l>(e{}));
	
template <typename newl, typename l, typename y, typename v, typename e>
constexpr auto _relabel(const typename AST<l>::template Binding<l, y, v, e>&)
{
	return typename AST<newl>::template Binding<newl, y, v, relabel<newl,l,e> >{};
}

template <typename newl, typename l, typename y, typename s, typename f>
constexpr auto _relabel(const typename AST<l>::template Expression<y, typename AST<l>::template FieldReference<s, f>>&)
{
	return typename AST<newl>::template Expression<y, typename AST<newl>::template FieldReference<relabel<newl,l,s>, f>>{};
}
  
template <typename newl, typename l, typename y, typename v>
constexpr auto _relabel(const typename AST<l>::template Expression<y, typename AST<l>::template VarReference<v>>&)
{
	return typename AST<newl>::template Expression<y, typename AST<newl>::template VarReference<v>>{};
}

template <typename newl, typename l, int i>
constexpr auto _relabel(const typename AST<l>::template Expression<int, typename AST<l>::template Constant<i>>&)
{
	return typename AST<newl>::template Expression<int, typename AST<newl>::template Constant<i> >{};
}

template <typename newl, typename l>
constexpr auto _relabel(const typename AST<l>::template Expression<tracker::Tombstone, typename AST<l>::template GenerateTombstone<>>&)
{
	return typename AST<newl>::template Expression<tracker::Tombstone, typename AST<newl>::template GenerateTombstone<>>{};
}


template <typename newl, typename l, typename y, char op, typename L, typename R>
constexpr auto _relabel(const typename AST<l>::template Expression<y, typename AST<l>::template BinOp<op, L, R>>&)
{
	return typename AST<newl>::template Expression<y, typename AST<newl>::template BinOp<op, relabel<newl,l,L>, relabel<newl,l,R>>>{};
}

template <typename newl, typename l, typename b, typename body>
constexpr auto _relabel(const typename AST<l>::template Statement<typename AST<l>::template Let<b, body>>)
{
	return typename AST<newl>::template Statement<typename AST<newl>::template Let<relabel<newl,l,b>, relabel<newl,l,body>>>{};
}

template <typename newl, typename l, typename b, typename body>
constexpr auto _relabel(const typename AST<l>::template Statement<typename AST<l>::template LetRemote<b, body>>&)
{
	return typename AST<newl>::template Statement<typename AST<newl>::template LetRemote<relabel<newl,l,b>, relabel<newl,l,body>>>{};
}

	template <typename newl, typename l, typename y, typename h>
constexpr auto _relabel(const typename AST<l>::template Expression<y,typename AST<l>::template IsValid<h>>)
{
	return typename AST<newl>::template Expression<y,typename AST<newl>::template IsValid<relabel<newl,l,h>>>{};
}

template <typename newl, typename l, typename oper_name, typename Hndl, typename... args>
constexpr auto _relabel(const typename AST<l>::template Statement<typename AST<l>::template Operation<oper_name,Hndl,args...>>)
{
	return typename AST<newl>::template Statement<typename AST<newl>::template Operation<oper_name,relabel<newl,l,Hndl>,relabel<newl,l,args>...>>{};
}

template <typename newl, typename l, typename y, typename oper_name, typename Hndl, typename... args>
constexpr auto _relabel(const typename AST<l>::template Expression<y,typename AST<l>::template Operation<oper_name,Hndl,args...>>&)
{
	return typename AST<newl>::template Expression<y,typename AST<newl>::template Operation<oper_name,relabel<newl,l,Hndl>,relabel<newl,l,args>...>>{};
}

	
template <typename newl, typename l, typename L, typename R>
constexpr auto _relabel(const typename AST<l>::template Statement<typename AST<l>::template Assignment<L, R>>&)
{
	return typename AST<newl>::template Statement<typename AST<newl>::template Assignment<relabel<newl,l,L>, relabel<newl,l,R>>>{};
}

template <typename newl, typename l, typename R>
constexpr auto _relabel(const typename AST<l>::template Statement<typename AST<l>::template Return<R>>&)
{
	return typename AST<newl>::template Statement<typename AST<newl>::template Return<relabel<newl,l,R>>>{};
}
  
template <typename newl, typename l, typename R>
constexpr auto _relabel(const typename AST<l>::template Statement<typename AST<l>::template AccompanyWrite<R>>)
{
	return typename AST<newl>::template Statement<typename AST<newl>::template AccompanyWrite<relabel<newl,l,R> > >{};
}

  template <typename newl, typename l, typename e>
constexpr auto _relabel(const typename AST<l>::template Statement<typename AST<l>::template WriteTombstone<e>>)
{
	return typename AST<newl>::template Statement<typename AST<newl>::template WriteTombstone<relabel<newl,l,e>>>{};
}

template <typename newl, typename l, char... var>
constexpr auto _relabel(const typename AST<l>::template Statement<typename AST<l>::template IncrementOccurance<String<var...>>>&)
{
	return typename AST<newl>::template Statement<typename AST<newl>::template IncrementOccurance<String<var...>>>{};
}

template <typename newl, typename l, char... var>
constexpr auto _relabel(const typename AST<l>::template Statement<typename AST<l>::template IncrementRemoteOccurance<String<var...>>>&)
{
	return typename AST<newl>::template Statement<typename AST<newl>::template IncrementRemoteOccurance<String<var...>>>{};
}


template <typename newl, typename l, typename c, typename t, typename e>
constexpr auto _relabel(const typename AST<l>::template Statement<typename AST<l>::template If<c, t, e>>&)
{
	return typename AST<newl>::template Statement<typename AST<newl>::template If<relabel<newl,l,c>, relabel<newl,l,t>, relabel<newl,l,e>>>{};
}

template <typename newl, typename l, typename c, typename t, char... name>
constexpr auto _relabel(const typename AST<l>::template Statement<typename AST<l>::template While<c, t, name...>>&)
{
	return typename AST<newl>::template Statement<typename AST<newl>::template While<relabel<newl,l,c>, relabel<newl,l,t>, name...>>{};
}

template <typename newl, typename l, typename t, char... name>
constexpr auto _relabel(const typename AST<l>::template Statement<typename AST<l>::template ForEach<t, name...>>&)
{
	return typename AST<newl>::template Statement<typename AST<newl>::template ForEach<relabel<newl,l,t>, name...>>{};
}

template <typename newl, typename l, typename... Seq>
constexpr auto _relabel(const typename AST<l>::template Statement<typename AST<l>::template Sequence<Seq...>>)
{
	return typename AST<newl>::template Statement<typename AST<newl>::template Sequence<relabel<newl,l,Seq>...>>{};
}

	template<typename newl, typename l, typename e>
	constexpr auto __relabel(e){
		return _relabel<newl,l>(e{});
	}
}
	namespace runnable_transaction {

		template <typename l, typename _returns, typename AST, typename reqs, typename provides, typename owns, typename passthrough>
		constexpr auto relabel(phase<l, _returns, AST, reqs, provides, owns, passthrough> a){
			return a;
		}

		template <typename l, typename _returns, typename AST, typename reqs, typename provides, typename owns, typename passthrough>
		constexpr auto relabel(phase<Label<PreEndorse<l> >, _returns, AST, reqs, provides, owns, passthrough> ){
			return phase<l, _returns, split_phase::relabel<l,Label<PreEndorse<l> >,AST>,
						 reqs,provides,owns,passthrough>{};
		}
		
		template<typename... phases>
		constexpr auto relabel(transaction<phases...>){
			return transaction<DECT(relabel(phases{}))... >{};
		}
	}
}
}
