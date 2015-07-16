#pragma once
#include "ConExpr.hpp"

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

template<typename T>
struct Not : public ConExpr<get_level<T>::value> {

	static_assert(is_ConExpr<T>::value,"Error: cannot negate non-expression");
	
	T v;
	Not(const T& t):v(t){}
	
	auto operator()() const {
		return !v();
	}

	BitSet<backend::HandleAbbrev> getReadSet() const {
		return v.getReadSet();
	}
};

template<typename T>
struct IsValid : public ConExpr<get_level<T>::value> {
	static_assert(backend::is_handle<T>::value,"error: IsValid designed for referential integrity of handles.");

	const T t;
	
	IsValid(const T &t):t(t){}
	
	bool operator()() const {
		//TODO: when handles re-design happens,
		//this should be one of the basic things
		//exposed at the handle level.
		return true;
	}

	auto getReadSet() const {
		backend::HandleAbbrev hb = t;
		BitSet<backend::HandleAbbrev> ret(hb);
		return ret;
	}
	
};

template<typename T>
auto isValid(const T &t){
	return IsValid<T>(t);
}
	
template<typename T, typename... Handles>
struct FreeExpr : public ConExpr<min_level<Handles...>::value > {
private:
	std::unique_ptr<const T> sto;
public:

	//todo: idea here is that only read-only things can be done to the handles
	//in this context.  Try to make that a reality please.
	std::unique_ptr<std::function<T ()> > f;
	std::unique_ptr<const BitSet<backend::HandleAbbrev> > rs;
	
	FreeExpr(int, std::function<T (const typename backend::extract_type<Handles>::type & ... )> f, Handles... h)
		:f(new std::function<T ()>([&,f,h...](){return f(h.get()...);})),
		 rs(new BitSet<backend::HandleAbbrev>(setify(h.abbrev()...)))
		{}

	T operator()(){
		if (!sto) sto.reset(new T((*f)()));
		return *sto;
	}

	BitSet<backend::HandleAbbrev> getReadSet() const {
		return *rs;
	}

	
	template<typename F>
	FreeExpr(F f, Handles... h):FreeExpr(0, convert(f), h...){}
};


template<typename T, typename... Handles>
T run_expr(FreeExpr<T, Handles...> fe){
	return fe();
}


#define free_expr1(T,a,e) FreeExpr<T,decltype(a)>([&](const typename extract_type<decltype(a)>::type &a){e},a)
#define free_expr2(T,a,b,e) FreeExpr<T,decltype(a),decltype(b)>([&](const typename extract_type<decltype(a)>::type &a, \
																	const typename extract_type<decltype(b)>::type &b){e},a,b)


