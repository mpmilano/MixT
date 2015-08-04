#pragma once


#include "utils.hpp"
#include <string>
#include <memory>
#include <tuple>
#include "ConStatement.hpp"
#include "args-finder.hpp"

typedef Level Level;

template<typename T, Level l>
struct ConExpr : public ConStatement<l> {};

template<Level l>
struct DummyConExpr : public ConExpr<void,l> {
	BitSet<HandleAbbrev> getReadSet() const {
		return BitSet<HandleAbbrev>();
	}
};

template<typename T, Level l>
constexpr bool is_ConExpr_f(const ConExpr<T,l>*){
	return true;
}

template<typename T>
constexpr bool is_ConExpr_f(const T*){
	return false;
}

template<typename Cls>
struct is_ConExpr : 
	std::integral_constant<bool, is_ConExpr_f(mke_p<Cls>()) || std::is_pod<decay<Cls> >::value>::type {};

template<typename Expr, restrict(is_ConExpr<Expr>::value && std::is_pod<Expr>::value)>
auto get_ReadSet(const Expr &){
	return BitSet<HandleAbbrev>();
}

template<typename T>
typename std::enable_if<is_ConStatement<T>::value && !std::is_pod<T>::value,
						BitSet<HandleAbbrev> >::type get_ReadSet(const T &ce){
	assert(&ce != nullptr);
	return ce.getReadSet();
}


template<typename T>
T run_expr(const T& t){
	static_assert(std::is_pod<T>::value,"Error: running non-POD not yet supported.  Store simpler things.");
	return t;
}

