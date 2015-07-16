#pragma once
#include "ConExpr.hpp"

//the level here is for referencing the temporary later.
//it's the endorsement check!
template<backend::Level l, typename T>
struct Temporary : public ConStatement<get_level<T>::value> {
	static_assert(is_ConExpr<T>::value,"Error: can only assign temporary the result of expressions");
	static_assert(l == backend::Level::causal ||
				  get_level<T>::value == backend::Level::strong,
		"Error: flow violation");
	const std::shared_ptr<const T> t;
	std::shared_ptr<const decltype((*t)())> res;
	Temporary(const T& t):t(new T(t)){}

	auto getReadSet() const {
		return t->getReadSet();
	}

	auto operator()() const {
		if (!res) res.reset((*t)());
		return true;
	}
};


template<backend::Level l, typename T>
auto make_temp(const T& t){
	return Temporary<l,T>(t);
}
