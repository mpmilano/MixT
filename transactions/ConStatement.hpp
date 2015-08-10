#pragma once

#include "utils.hpp"
#include "tuple_extras.hpp"
#include <string>
#include <memory>
#include <tuple>
#include <iostream>
#include "../BitSet.hpp"
#include "Handle.hpp"


template<Level l>
struct ConStatement {
	static constexpr Level level = l;
	//virtual BitSet<HandleAbbrev> getReadSet() const = 0;
};

template<typename>
struct all_declarations_str;

template<typename T>
using all_declarations = typename all_declarations_str<T>::type;

template<typename... T>
struct all_declarations_str<std::tuple<T...> > {
	using type = Cat<all_declarations<T>... > ;
};


template<typename Cls>
struct is_ConStatement : 
	std::integral_constant<bool, 
						   std::is_base_of<ConStatement<Level::causal>,Cls>::value ||
						   std::is_base_of<ConStatement<Level::strong>,Cls>::value
						   >::type {};


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

template<typename T, restrict(! (is_ConStatement<T>::value || is_handle<T>::value || is_tuple<T>::value))>
constexpr Level get_level_f(const T*){
	return Level::strong;
}

template<typename T>
struct get_level : std::integral_constant<Level, get_level_f(mke_p<T>())>::type {};

template<typename... T>
struct min_level : std::integral_constant<Level,
										  (exists(is_causal(get_level<T>::value)...) ?
										   Level::causal :
										   Level::strong)>::type {};

template<typename... T>
struct min_level<std::tuple<T...> > : min_level<T...> {};


template<typename... T>
struct max_level : std::integral_constant<Level,
										  (exists(is_strong(get_level<T>::value)...) ?
										   Level::strong :
										   Level::causal)>::type {};

template<typename... T>
struct max_level<std::tuple<T...> > : max_level<T...> {};


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

