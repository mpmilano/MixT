#pragma once
#include "ConExpr.hpp"
#include "../BitSet.hpp"

template<Level l, int i>
class CSInt : public ConExpr<int,l>, public std::integral_constant<int,i>::type {
public:
	CSInt(){}

	BitSet<HandleAbbrev> getReadSet() const {
		return BitSet<HandleAbbrev>();
	}

	CONNECTOR_OP

	constexpr int operator()(const Store &) const {
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

	constexpr T causalCall(Store& cache, const Store&) const {
		cache.insert(this->id,val);
		return val;
	}

	constexpr T strongCall(Store& cache, const Store&) const {
		cache.insert(this->id,val);
		return val;
	}

	static_assert(!std::is_scalar<CSConstant<l,T> >::value,"Static assert failed");
	
};

template<unsigned long long, Level l, typename T>
auto find_usage(const CSConstant<l,T> &t){
	return nullptr;
}

template<unsigned long long ID, Level l, typename T>
struct contains_temporary<ID, CSConstant<l,T> > : std::false_type {};

template<typename T, restrict(std::is_scalar<T>::value)>
auto wrap_constants(const T &t){
	return CSConstant<Level::strong,T>{t};
}

template<typename T>
std::enable_if_t<!std::is_scalar<T>::value, T> wrap_constants(const T& t){
	return t;
}

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

	BitSet<HandleAbbrev> getReadSet() const {
		return v.getReadSet();
	}

	bool causalCall(Store& cache, const Store& s) const {

		if (cache.contains(this->id) ) return cache.get<bool>(this->id);
		else {
			cache.insert(this->id,!v.causalCall(cache,s));
			return !v.causalCall(cache,s);
		}
	}
	
	auto strongCall(Store& cache, const Store &s) const {
		std::integral_constant<bool,get_level<T>::value == Level::strong>*
			choice{nullptr};
		return strongCall(cache,s,choice);
	}

	bool strongCall(Store& cache, const Store& s, std::true_type*) const {
		bool ret = !v.strongCall(cache,s);
		cache.insert(this->id,ret);
		return ret;
	}

	void strongCall(Store &cache, const Store& s, std::false_type*) const {
		v.strongCall(cache,s);
	}


	template<typename i2>
	friend std::ostream & operator<<(std::ostream &os, const Not<i2>&);
};

template<unsigned long long ID, typename T>
struct contains_temporary<ID, Not<T> > : contains_temporary<ID,T> {};


template<unsigned long long ID, typename T>
auto find_usage(const Not<T> &t){
	return find_usage<ID>(t.v);
}


//TODO: figure out why this needs to be here
template<typename T>
struct is_ConExpr<Not<T> > : std::true_type {};

template<typename i2>
std::ostream & operator<<(std::ostream &os, const Not<i2>& n){
	return os << "!" << n.v;
}


template<typename T>
Not<T> neg(const T& t){
	return Not<T>(t);
}

template<Level l>
DummyConExpr<l> neg(const DummyConExpr<l>& e){
	return e;
}

template<typename T>
std::enable_if_t<is_ConExpr<T>::value, Not<T> >  operator!(const T &t){
	return neg(t);
}

template<typename T>
struct IsValid : public ConExpr<bool, get_level<T>::value> {
	static_assert(is_handle<decltype(run_ast_causal(mke_store(),mke_store(),mke<T>()))>::value,"error: IsValid designed for referential integrity of handles.");

	const T t;
	
	IsValid(const T &t):t(t){}

	bool causalCall(Store& cache, const Store& s) const {
		if (cache.contains(this->id) ) return cache.get<bool>(this->id);
		else {
			bool ret = run_ast_causal(cache,s,t).isValid();
			cache.insert(this->id,ret);
			return ret;
		}
	}
	
	auto strongCall(Store& cache, const Store &s) const {
		std::integral_constant<bool,get_level<T>::value == Level::strong>*
			choice{nullptr};
		return strongCall(cache,s,choice);
	}

	bool strongCall(Store& cache, const Store&s, std::true_type*) const {
		bool ret = run_ast_strong(cache,s,t).isValid();
		cache.insert(this->id,ret);
		return ret;
	}

	void strongCall(Store &cache, const Store& s, std::false_type*) const {
		run_ast_strong(cache,s,t);
	}


	auto getReadSet() const {
		HandleAbbrev hb = t;
		BitSet<HandleAbbrev> ret(hb);
		return ret;
	}

	template<typename T2>
	friend std::ostream & operator<<(std::ostream &os, const IsValid<T2>&);
};

template<typename T>
auto make_isValid(const T&t){ return IsValid<T>{t}; }


template<unsigned long long ID, typename T>
struct contains_temporary<ID, IsValid<T> > : contains_temporary<ID,T> {};


template<unsigned long long ID, typename T>
auto find_usage(const IsValid<T> &t){
	return find_usage<ID>(t.t);
}


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
