#pragma once
#include "ConExpr.hpp"
#include "Temporary.hpp"
#include "../BitSet.hpp"

template<Level l, int i>
class CSInt : public ConExpr<int,l>, public std::integral_constant<int,i>::type {
public:
	CSInt(){}

	BitSet<HandleAbbrev> getReadSet() const {
		return BitSet<HandleAbbrev>();
	}

	CONNECTOR_OP

	constexpr auto operator()(Store &) const {
		return i;
	}
	
	template<Level l2, int i2>
	friend std::ostream & operator<<(std::ostream &os, const CSInt<l2,i2>&);
};

template<Level l, int i>
std::ostream & operator<<(std::ostream &os, const CSInt<l,i>&){
	return os << i;
}

template<Level l, int i>
constexpr bool verify_compilation_complete(const CSInt<l,i>*)
{return true; }

template<Level l, typename T>
class CSConstant : public ConExpr<T,l> {
public:

	const T val;
	
	CSConstant(const T& t):val(t){}

	BitSet<HandleAbbrev> getReadSet() const {
		return BitSet<HandleAbbrev>();
	}

	CONNECTOR_OP

	constexpr auto operator()(Store &) const {
		return val;
	}
	
};

REPLACEME_OK(CSConstant)

//TODO: figure out why this needs to be here
template<Level l, typename T>
struct is_ConExpr<CSConstant<l,T> > : std::true_type {};

template<Level l, typename i>
std::ostream & operator<<(std::ostream &os, const CSConstant<l,i>& c){
	return os << c.val;
}


template<Level l, int i>
constexpr bool is_base_CS_f(const CSInt<l,i>* ){
	return true;
}

template<typename T>
struct Not : public ConExpr<bool, get_level<T>::value> {

	static_assert(is_ConExpr<T>::value,"Error: cannot negate non-expression");
	
	T v;
	Not(const T& t):v(t){}
	
	auto operator()(Store &s) const {
		return !v(s);
	}

	BitSet<HandleAbbrev> getReadSet() const {
		return v.getReadSet();
	}

	template<typename i2>
	friend std::ostream & operator<<(std::ostream &os, const Not<i2>&);
};

//TODO: figure out why this needs to be here
template<typename T>
struct is_ConExpr<Not<T> > : std::true_type {};

template<typename i2>
std::ostream & operator<<(std::ostream &os, const Not<i2>& n){
	return os << "!" << n.v;
}


template<typename T>
Not<T> make_not(const T& t){
	return Not<T>(t);
}

template<Level l>
DummyConExpr<l> make_not(const DummyConExpr<l>& e){
	return e;
}

template<typename T>
struct IsValid : public ConExpr<bool, get_level<T>::value> {
	static_assert(is_handle<T>::value,"error: IsValid designed for referential integrity of handles.");

	const T t;
	
	IsValid(const T &t):t(t){}
	
	bool operator()(const Store &) const {
		//TODO: when handles re-design happens,
		//this should be one of the basic things
		//exposed at the handle level.
		return true;
	}

	auto getReadSet() const {
		HandleAbbrev hb = t;
		BitSet<HandleAbbrev> ret(hb);
		return ret;
	}

	template<typename T2>
	friend std::ostream & operator<<(std::ostream &os, const IsValid<T2>&);
};

template<typename T2>
std::ostream & operator<<(std::ostream &os, const IsValid<T2> &t){
	return os << "isValid(" << t.t << ")";
}


template<typename T>
auto isValid(const T &t){
	return IsValid<T>(t);
}
	

//TODO: figure out why this needs to be here
template<typename T>
struct is_ConExpr<IsValid<T> > : std::true_type {};
