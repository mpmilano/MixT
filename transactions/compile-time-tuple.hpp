#pragma once
#include <type_traits>
#include "restrict.hpp"
#include "utils.hpp"

namespace ct {

	template<typename A, typename... B>
	struct tuple;

	template<int i, typename A, typename... B>
	constexpr auto get(const typename std::enable_if<(i > 0) && (sizeof...(B) > 0), tuple<A,B...> >::type &t){
		return get<i-1>(t.rest);
	}

	template<int i, typename A, typename... B>
	constexpr typename std::enable_if<i == 0 && (sizeof...(B) > 0), A>::type
	get(const tuple<A,B...> &t){
		return t.a;
	}

	template<int i, typename A, restrict(i == 0)>
	constexpr auto get(const tuple<A> &t){
		return t.a;
	}

	template<typename A, typename... B>
	struct tuple {
		const A a;
		const tuple<B...> rest;

		template<int i>
		constexpr auto get() const {
			static_assert(i < (sizeof...(B) + 1),"Error: index out of range!");
			return ct::get<i,A,B...>(*this);
		}
	};

	template<typename A>
	struct tuple<A> {
		const A a;
		template<int i = 0, restrict(i == 0)>
		constexpr A get(){
			static_assert(i == 0,"Error: index out of range!");
			return a;
		}
	};

	template<typename A>
	constexpr auto make_tuple(const A &a) {
		return tuple<A>{a};
	}
	
	template<typename A, typename ... B>
	constexpr typename std::enable_if<(sizeof...(B) > 0),tuple<A,B...> >::type
	make_tuple(const A &a, const B & ... b){
		return tuple<A,B...>{a, make_tuple(b...)};
	}
}


