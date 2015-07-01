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


template<typename Cls>
struct is_ConExpr : 
	std::integral_constant<bool, 
						   std::is_base_of<ConExpr<Level::causal>,Cls>::value ||
						   std::is_base_of<ConExpr<Level::strong>,Cls>::value ||
						   std::is_pod<Cls>::value
						   >::type {};

template<typename T, typename... Handles>
struct FreeExpr : public ConExpr<min_level<Handles...>::value > {
private:
	std::unique_ptr<const T> sto;
public:

	//todo: idea here is that only read-only things can be done to the handles
	//in this context.  Try to make that a reality please.
	std::function<T ()> f;
	
	FreeExpr(int, std::function<T (const typename extract_type<Handles>::type & ... )> f, Handles... h)
		:f([&,f,h...](){return f(h.get()...);}) {}

	T operator()(){
		if (!sto) sto.reset(new T(f()));
		return *sto;
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


