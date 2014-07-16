#pragma once
#include "Backend.h"
#include <functional>
#include <list>
#include <utility>
#include <tuple>
#include <memory>

#define CONST_LVALUE(x)  typename std::add_lvalue_reference<typename std::add_const<x>::type>::type
#define RVALUE(x) typename std::add_rvalue_reference<x>::type

namespace interior_util {
	

	template< std::size_t I = 0, typename... Ts >
	inline typename std::enable_if< I == (sizeof... (Ts)) - 1, std::tuple< Ts... >& >::type 
	copy_tail (std::tuple< Ts... >& tl)
	{
		return std::make_tuple(std::get<I> (tl));
	}

	template< std::size_t I = 0, typename... Ts >
	inline typename std::enable_if< I != (sizeof... (Ts)) - 1, std::tuple< Ts... >& >::type 
	copy_tail (std::tuple< Ts... >& tl)
	{
		return std::tuple_cat( std::make_tuple(std::get< I >(tl)), copy_tail<I + 1, Ts...>(tl)); 
	}
	

	template<typename Head, typename... Tail>
	std::tuple<Tail...> tuple_tail(std::tuple<Head, Tail...> &rest) {
		return copy_tail<1, Tail...>(rest);
	}

	template<typename R, typename Head, typename... Tail >
	R call_with_args (std::function<R (Head, Tail...)> &f, const std::tuple<Head&&,  RVALUE(Tail)...> args) {
		if (std::tuple_size<decltype(args)>::value > 1)
			return call_with_args(std::bind(f, std::get<0>(args)),tuple_tail(args));
		else 
			return f(std::get<0>(args));
	}
}

namespace tester {
	
#if 0
	template<typename R, typename IR, backend::Level L, typename... A>
	class Fuzz {
	private:
		backend::DataStore<L> &ds;
		std::list<std::pair<std::function<IR (std::list<R>)> ,
				    std::function<R (backend::DataStore<L> &) > > >test_funs;
	public:
		Fuzz(backend::DataStore<L> &ds):ds(ds),test_funs(std::list<std::pair<std::function<IR (std::list<R>)> ,
				    std::function<R (backend::DataStore<L> &) > > >()){}


		void registerTestFunction(std::function<IR (std::list<R>)> &check_invariants,
					  std::function<R (backend::DataStore<L> &, A... )> &tf, 
					  CONST_LVALUE(A)... extra_args){

			auto extra_args_tuple = std::forward_as_tuple(extra_args...);
			std::function<R (backend::DataStore<L> &) > &&storeargs = 
				[&] (backend::DataStore<L> &ds_prime) 
				{return interior_util::call_with_args(std::bind(tf, ds_prime), extra_args_tuple); };
			test_funs.push_back(std::make_pair(check_invariants, storeargs));
		}

	};

	template<typename R, typename IR, backend::Level L, typename... A>
	Fuzz<R,IR,L,A...> registerTestFunction(backend::DataStore<L> &ds,
					       std::function<IR (std::list<R>)> &check_invariants,
					       std::function<R (backend::DataStore<L> &, A... )> &tf,
					       CONST_LVALUE(A)... extra_args){
		auto &&ret = Fuzz<R,IR,L,A...>(ds);
		ret.registerTestFunction(check_invariants, tf, extra_args...);
		return ret;
	}

#endif
#if 1
	template<typename R, typename IR, backend::Level L, typename... A>
	IR registerTestFunction(backend::DataStore<L> &ds,
				std::function<IR (std::list<R>)> &check_invariants,
				std::function<R (backend::DataStore<L> &, A... )> &tf, 
				CONST_LVALUE(A)... extra_args ){
		std::list<R> rlist;
		using namespace interior_util;
		using namespace std::placeholders;
		//auto newargs = std::tuple_cat(std::forward_as_tuple(ds), std::forward_as_tuple(extra_args...)); 
		//auto tmp = call_with_args<R, backend::DataStore<L>&> (tf, newargs);
		auto tmp1 = std::bind(tf, _1, std::move(extra_args)...);
		auto tmp = tmp1(ds);
		rlist.push_back(tmp);
		return check_invariants(rlist);
	}
#endif

}
