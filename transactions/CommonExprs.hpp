#pragma once
#include "ConExpr.hpp"
#include "../BitSet.hpp"


template<Level l, typename T>
class CSConstant : public ConExpr<T,l> {
public:

	const T val;

	const int id = gensym();
	
	CSConstant(const T& t):val(t){}

	constexpr T causalCall(Store& cache, const Store&) const {
		cache.insert(this->id,val);
		return val;
	}

	constexpr T strongCall(Store& cache, const Store&) const {
		cache.insert(this->id,val);
		return val;
	}

	std::tuple<> handles() const {
		static std::tuple<> ret;
		return ret;
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

template<typename T>
struct Not : public ConExpr<bool, get_level<T>::value> {

	static_assert(is_ConExpr<T>::value,"Error: cannot negate non-expression");

	const int id = gensym();
	T v;
	Not(const T& t):v(t){}

	auto handles() const {
		return v.handles();
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
	static_assert(is_handle<run_result<T> >::value,"error: IsValid designed for referential integrity of handles.");

	const T t;
	const int id = gensym();
	
	IsValid(const T &t):t(t){}

	auto handles() const {
		return ::handles(t);
	}

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


template<typename T>
auto isValid(const T &t){
	return IsValid<T>(t);
}
	

//TODO: figure out why this needs to be here
template<typename T>
struct is_ConExpr<IsValid<T> > : std::true_type {};
