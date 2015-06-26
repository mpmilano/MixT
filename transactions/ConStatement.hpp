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

template<typename... Args>
constexpr bool is_tuple_f(std::tuple<Args...>*){
	return forall(is_ConStatement<Args>::value...);
}

template<typename F>
struct is_cs_tuple : std::integral_constant<bool,
										 is_tuple_f((F*) nullptr)
										 >::type {};

