#pragma once

#include "../Backend.hpp"
#include "utils.hpp"
#include <string>
#include <memory>
#include <tuple>
#include "../extras"
#include "ConStatement.hpp"


typedef backend::Level Level;

template<Level l>
struct ConExpr : public ConStatement<l> {};


template<typename Cls>
struct is_ConExpr : 
	std::integral_constant<bool, 
						   std::is_base_of<ConExpr<Level::causal>,Cls>::value ||
						   std::is_base_of<ConExpr<Level::strong>,Cls>::value
						   >::type {};
