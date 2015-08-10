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
	typedef std::integral_constant<Level, Level::strong> pc;

	
	template<typename T>
	typename std::enable_if<is_ConStatement<T>::value ,
							TransactionBuilder<Cat<Statements, std::tuple<T> >,
											   Cat<Vars,all_declarations<T> > > >::type
	operator/(const T &t){
		TransactionBuilder<Cat<Statements, std::tuple<T> >,
						   Cat<Vars,all_declarations<T> > >
			r{std::tuple_cat(curr,std::make_tuple(t))};
		return r;
	}
};

//the default append function.
//T typecheck is handled by operator/
template<typename CurrBuilder, typename T>
auto append(const CurrBuilder &pb, const T &t){
	NAME_CHECK(CurrBuilder::vars,T);
	return pb / t;
}
