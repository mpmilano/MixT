#pragma once
#include "Basics.hpp"
#include "type_utils.hpp"
#include "ConStatement.hpp"
#include "WhileBuilder.hpp"

//this makes sure the expression (expr)
//only references temporaries defined in the tuple
//Names

template<typename Statements>
struct TransactionBuilder : public Base_Builder{
	const Statements curr;
	typedef std::integral_constant<Level, Level::strong> pc;

	TransactionBuilder(const Statements &c):curr(c){}
	TransactionBuilder(){}
	
	template<typename T>
	std::enable_if_t<is_ConStatement<T>::value ,
			  TransactionBuilder<Cat<Statements, std::tuple<T> > > >
	operator/(const T &t) const {
		TransactionBuilder<Cat<Statements, std::tuple<T> > >
			r{std::tuple_cat(curr,std::make_tuple(t))};
		return r;
	}
};

//the default append function.
//T typecheck is handled by operator/
template<typename CurrBuilder, typename T>
auto append(const CurrBuilder &pb, const T &t){
	return pb / t;
}

template<typename T>
struct Clobber {
	const T t;
};

template<typename T>
auto clobber(const T &t){
	Clobber<T> r{t};
	return r;
}

template<typename CurrBuilder, typename T>
auto append(const CurrBuilder &pb, const Clobber<T> &ct){
	return ct.t;
}

