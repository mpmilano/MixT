#pragma once
#include "Basics.hpp"
#include "type_utils.hpp"
#include "ConStatement.hpp"

//this makes sure the expression (expr)
//only references temporaries defined in the tuple
//Names
#define NAME_CHECK(Names,expr) 

template<typename Statements, typename Vars>
struct TransactionBuilder {
	const Statements curr;
	typedef Vars vars;
	static constexpr Level pc = Level::strong;

	template<typename T>
	auto operator/(const type_check<is_ConStatement, T> &t){
		TransactionBuilder<Cat<Statements, std::tuple<T> >,
						   Cat<Vars,all_declarations<T> > >
			r{std::tuple_cat(curr,std::make_tuple(t))};
		return r;
	}
};

//the default append function.
template<typename CurrBuilder, typename T>
auto append(const CurrBuilder &pb, const type_check<is_ConStatement, T> &t){
	NAME_CHECK(CurrBuilder::vars,T);
	return pb / t;
}
