#pragma once

#include "../Backend.hpp"
#include "utils.hpp"
#include <string>
#include <memory>
#include <tuple>
#include "../extras"


typedef backend::Level Level;

template<Level l>
struct ConStatement {

};

template<typename A>
constexpr bool is_tuple_f(A*){
	return false;
}

template<typename Cls>
struct is_ConStatement : 
	std::integral_constant<bool, 
						   std::is_base_of<ConStatement<Level::causal>,Cls>::value ||
						   std::is_base_of<ConStatement<Level::strong>,Cls>::value
						   >::type {};

template<typename... Args>
constexpr bool is_tuple_f(std::tuple<Args...>*){
	return forall(is_ConStatement<Args>::value...);
}

template<typename F>
struct is_cs_tuple : std::integral_constant<bool,
										 is_tuple_f((F*) nullptr)
										 >::type {};

template<Level l>
constexpr Level get_level_f(const ConStatement<l>*){
	return l;
}

template<typename T>
struct get_level : std::integral_constant<Level, get_level_f((T*) nullptr)>::type {};
