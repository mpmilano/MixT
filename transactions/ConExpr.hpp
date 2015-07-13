#pragma once

#include "../Backend.hpp"
#include "utils.hpp"
#include <string>
#include <memory>
#include <tuple>
#include "../extras"
#include "ConStatement.hpp"
#include "args-finder.hpp"

typedef backend::Level Level;

template<Level l>
struct ConExpr : public ConStatement<l> {};

template<Level l, int i>
class CSInt : public ConExpr<l>, public std::integral_constant<int,i>::type {
public:
	CSInt(){}

	BitSet<backend::HandleAbbrev> getReadSet() const {
		return BitSet<backend::HandleAbbrev>();
	}

	CONNECTOR_OP
	
	template<Level l2, int i2>
	friend std::ostream & operator<<(std::ostream &os, const CSInt<l2,i2>&);
};

template<Level l, int i>
std::ostream & operator<<(std::ostream &os, const CSInt<l,i>&){
	return os << i;
}

template<Level l, int i>
constexpr bool is_base_CS_f(const CSInt<l,i>* ){
	return true;
}


template<Level l>
struct DummyConExpr : public ConExpr<l> {
	BitSet<backend::HandleAbbrev> getReadSet() const {
		return BitSet<backend::HandleAbbrev>();
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
	return BitSet<backend::HandleAbbrev>();
}

template<Level l>
auto get_ReadSet(const ConExpr<l> &ce){
	return ce.getReadSet();
}



template<typename T, typename... Handles>
struct FreeExpr : public ConExpr<min_level<Handles...>::value > {
private:
	std::unique_ptr<const T> sto;
public:

	//todo: idea here is that only read-only things can be done to the handles
	//in this context.  Try to make that a reality please.
	std::function<T ()> f;
	const BitSet<backend::HandleAbbrev> rs;
	
	FreeExpr(int, std::function<T (const typename backend::extract_type<Handles>::type & ... )> f, Handles... h)
		:f([&,f,h...](){return f(h.get()...);}),
		 rs(setify(h.abbrev()...))
		{}

	T operator()(){
		if (!sto) sto.reset(new T(f()));
		return *sto;
	}

	BitSet<backend::HandleAbbrev> getReadSet() const {
		return rs;
	}

	
	template<typename F>
	FreeExpr(F f, Handles... h):FreeExpr(0, convert(f), h...){}
};


template<typename T, typename... Handles>
T run_expr(FreeExpr<T, Handles...> fe){
	return fe();
}

template<typename T>
T run_expr(const T& t){
	static_assert(std::is_pod<T>::value,"Error: running non-POD not yet supported.  Store simpler things.");
	return t;
}


#define free_expr1(T,a,e) FreeExpr<T,decltype(a)>([&](const typename extract_type<decltype(a)>::type &a){e},a)
#define free_expr2(T,a,b,e) FreeExpr<T,decltype(a),decltype(b)>([&](const typename extract_type<decltype(a)>::type &a, \
																	const typename extract_type<decltype(b)>::type &b){e},a,b)


