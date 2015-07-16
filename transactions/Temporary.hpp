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
	const T t;
	Temporary(const T& t):t(t){}

	auto getReadSet() const {
		return t.getReadSet();
	}

	auto operator()(Store &s) const {
		if (!s.contains(t /* TODO: uniqueness*/))
			s[t/*...*/] = t();
		return s.at(t /*again uniqueness */);
	}
};


template<backend::Level l, typename T>
auto make_temp(const T& t){
	return Temporary<l,T>(t);
}

template<backend::Level, backend::Level l>
auto make_temp(const DummyConExpr<l>& r){
	return r;
}

