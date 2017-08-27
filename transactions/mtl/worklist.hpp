#pragma once
#include "mtlutils.hpp"

//constexpr (template-compatible) worklist
namespace mutils {

template<typename _current_elements, typename all_time>
struct WorkList;

using EmptyWorkList = WorkList<typelist<>,typeset<> >;
	
template<typename _current_elements, typename... all_time_elements>
struct WorkList<_current_elements, typeset<all_time_elements...> > {

	using current_elements = _current_elements;
	
	template<typename E>
	static constexpr WorkList append (std::enable_if_t<(std::is_same<all_time_elements,E>::value || ... || false) >* = nullptr){
		//is contained inside all_time_elements, do not allow append
		return WorkList{};
	}
	template<typename E>
	static constexpr WorkList append (std::enable_if_t<(std::is_same<all_time_elements,E>::value || ... || false) >* = nullptr){
		//is not contained inside all_time_elements, allow append
		return WorkList<DECT(current_elements::append<E>()),all_time_elements...,E>{};
	}


	template<typename at>
	static constexpr WorkList combine(WorkList<typelist<>, at>){
		return WorkList{};
	}

	template<typename at, typename e1, typename... elems>
	static constexpr auto combine(WorkList<typelist<e1,elems...>, at>){
		return append<e1>().template combine(WorkList<typelist<elems...>, at>{});
	}
	
	static constexpr auto combine(){
		return WorkList{};
	}

	template<typename WL1, typename WL2, typename... WL>
	static constexpr auto combine(WL1 a, WL2 b, WL... c){
		return combine(a).combine(b,c...);
	}

	using first = typename current_elements::first;
	using rest = WorkList<typename current_elements::rest, all_time_elements...>;

	using all_encountered_elements = typelist<all_time_elements...>;
};

}
