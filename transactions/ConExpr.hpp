#pragma once


#include "utils.hpp"
#include <string>
#include <memory>
#include <tuple>
#include "ConStatement.hpp"
#include "args-finder.hpp"

typedef Level Level;

template<Level l>
struct ConExpr : public ConStatement<l> {};

template<Level l>
struct DummyConExpr : public ConExpr<l> {
	BitSet<HandleAbbrev> getReadSet() const {
		return BitSet<HandleAbbrev>();
	}
};

template<typename Cls>
struct is_ConExpr : 
	std::integral_constant<bool, 
						   std::is_base_of<ConExpr<Level::causal>,Cls>::value ||
						   std::is_base_of<ConExpr<Level::strong>,Cls>::value ||
						   std::is_pod<Cls>::value
						   >::type {};

template<typename Expr, restrict(is_ConExpr<Expr>::value && std::is_pod<Expr>::value)>
auto get_ReadSet(const Expr &){
	return BitSet<HandleAbbrev>();
}

template<typename T>
typename std::enable_if<is_ConExpr<T>::value && !std::is_pod<T>::value,
						BitSet<HandleAbbrev> >::type get_ReadSet(const T &ce){
	assert(&ce != nullptr);
	return ce.getReadSet();
}


template<typename T>
T run_expr(const T& t){
	static_assert(std::is_pod<T>::value,"Error: running non-POD not yet supported.  Store simpler things.");
	return t;
}

