#pragma once
#include "Temporary.hpp"

template<typename Var, typename Expr>
struct Assignment : public ConStatement<get_level<Expr>::value>{
	const Expr e;
	static_assert(
		can_flow(get_level<Expr>::value, get_level<Var>::value),
		"Error: assignment to strong member from causal expression!"
		);
};

template<unsigned long long ID, Level l, typename T, typename Temp, typename E,
		 restrict((std::is_same<Temporary<ID,l,T>, Temp>::value))>
auto operator<<(const RefTemporary<ID,l,T,Temp>&, const E &e){
	return Assignment<Temp,E>{e};
}

template<typename Var, typename Expr>
auto find_usage(const Assignment<Var,Expr> &rt){
	return find_usage(rt.e);
}
