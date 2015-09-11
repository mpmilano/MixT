#pragma once

#include "utils.hpp"
#include "tuple_extras.hpp"
#include <string>
#include <memory>
#include <tuple>
#include <iostream>
#include "../BitSet.hpp"
#include "Handle.hpp"
#include "Store.hpp"

struct GCS {};

template<Level l>
struct ConStatement : public GCS{
	static constexpr Level level = l;
};

template<typename Arg, typename Acc>
using contains_temporary_aggr = 
	std::pair<Left<Acc>, std::integral_constant<bool, (contains_temporary<Left<Acc>::value, Arg>::value || Right<Acc>::value) > >;

template<unsigned long long key, typename Lst>
using contains_temp_fold = 
	Right<fold_types<contains_temporary_aggr, Lst,
					 std::pair<std::integral_constant<unsigned long long, key>,
							   std::true_type> > >;

template<typename Cls>
struct is_ConStatement : 
	std::integral_constant<bool, std::is_base_of<GCS,Cls>::value>::type {};


template<Level l>
constexpr Level get_level_f(const ConStatement<l>*){
	return l;
}

template<Level l>
constexpr Level get_level_f(const std::integral_constant<Level, l>*){
	return l;
}

template< Level l, HandleAccess ha, typename T>
constexpr Level get_level_f(const Handle<l,ha,T>*){
	return l;
}


template<typename A>
constexpr bool is_tuple_f(A*){
	return false;
}

template<typename... Args>
constexpr bool is_tuple_f(std::tuple<Args...>*){
	return forall(is_ConStatement<Args>::value...);
}

template<typename F>
struct is_cs_tuple : std::integral_constant<bool,
										 is_tuple_f((F*) nullptr)
										 >::type {};

template<Level l>
using choose_strong = typename std::integral_constant<bool, l == Level::strong>::type*;

template<Level l>
using choose_causal = typename std::integral_constant<bool, l == Level::causal>::type*;

template<typename... CS>
auto call_all_causal(Store &cache, Store &st, const std::tuple<CS...> &t){
	bool check = fold(t,[&cache,&st](const auto &e, bool b)
					  {return b && e.causalCall(cache,st);},true);
	assert(check);
	return check;
}


template<typename... CS>
auto call_all_strong(Store &cache, Store &st, const std::tuple<CS...> &t){
	//TODO: better error propogation please.
	bool check = fold(t,[&cache,&st](const auto &e, bool b)
					  {e.strongCall(cache,st); return true;},true);
	assert(check);
	return check;
}

template<typename... T>
auto stmt_handles(const std::tuple<T...> &cs){
	return fold(cs,[](const auto &e, const auto &acc){
			return std::tuple_cat(e.handles(),acc);
		},
		std::tuple<>());
}

