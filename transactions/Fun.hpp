#pragma once
#include "ConStatement.hpp"
#include "BaseCS.hpp"
#include "ConExpr.hpp"
#include "Seq.hpp"
#include <iostream>


template<typename Arg, typename Strong, typename Weak>
class Fun;

#define handle_level backend::handle_level

#define fun_concept(Arg,Strong,Weak) ( \
																		\
		is_ConExpr<Arg>::value && is_cs_tuple<Strong>::value			\
		&& is_cs_tuple<Weak>::value										\
																		\
		)

