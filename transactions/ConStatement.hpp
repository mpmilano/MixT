#pragma once

#include "../Backend.hpp"
#include "utils.hpp"
#include <string>
#include <memory>
#include <tuple>
#include "../BitSet.hpp"
#include "../extras"


typedef backend::Level Level;

template<Level l>
struct ConStatement {
	//virtual BitSet<backend::HandleAbbrev> getReadSet() const = 0;
};

template<typename Cls>
struct is_ConStatement : 
	std::integral_constant<bool, 
						   std::is_base_of<ConStatement<backend::Level::causal>,Cls>::value ||
						   std::is_base_of<ConStatement<backend::Level::strong>,Cls>::value
						   >::type {};


template<backend::Level l>
constexpr backend::Level get_level_f(const ConStatement<l>*){
	return l;
}

template<backend::Client_Id id, backend::Level l, backend::HandleAccess ha, typename T>
constexpr backend::Level get_level_f(const backend::DataStore::Handle<id,l,ha,T>*){
	return l;
}

template<typename T, restrict(! (is_ConStatement<T>::value || backend::is_handle<T>::value || is_tuple<T>::value))>
constexpr backend::Level get_level_f(const T*){
	return backend::Level::strong;
}

template<typename T>
struct get_level : std::integral_constant<backend::Level, get_level_f((T*) nullptr)>::type {};

template<typename... T>
struct min_level : std::integral_constant<backend::Level,
										  (exists(is_causal(get_level<T>::value)...) ?
										   backend::Level::causal :
										   backend::Level::strong)>::type {};

template<typename... T>
struct min_level<std::tuple<T...> > : min_level<T...> {};


template<typename... T>
struct max_level : std::integral_constant<backend::Level,
										  (exists(is_strong(get_level<T>::value)...) ?
										   backend::Level::strong :
										   backend::Level::causal)>::type {};

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
