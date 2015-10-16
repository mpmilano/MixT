#pragma once
#include "ConExpr.hpp"
#include "../BitSet.hpp"
#include "Context.hpp"


template<Level l, typename T>
class CSConstant : public ConExpr<T,l> {
public:

	const T val;

	const int id = gensym();
	
	CSConstant(const T& t):val(t){}
	CSConstant(const CSConstant& cs):val(cs.val){}

	constexpr T causalCall(CausalCache& cache, const CausalStore&) const {
		cache.insert(this->id,val);
		return val;
	}

	constexpr T strongCall(StrongCache& cache, const StrongStore&) const {
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
	return CSConstant<Level::undef,T>{t};
}

template<typename T, restrict(std::is_scalar<T>::value)>
auto import(const T& t){
	return wrap_constants(t);
}

template<typename T>
std::enable_if_t<!std::is_scalar<T>::value, T> wrap_constants(const T& t){
	return t;
}

//TODO: figure out why this needs to be here
template<Level l, typename T>
struct is_ConExpr<CSConstant<l,T> > : std::true_type {};

template<typename T, typename V>
struct Sum : public ConExpr<bool, min_level<T,V>::value> {

	static_assert(is_ConExpr<T>::value,"Error: cannot sum non-expression");
	static_assert(is_ConExpr<V>::value,"Error: cannot sum non-expression");

	const int id = gensym();
	T l;
	V r;

	using res_t = decltype((std::declval<run_result<T> >()) +
						   std::declval<run_result<V> >());
	
	Sum(const T& l, const V& r):l(l),r(r){}
	Sum(const Sum& s):l(s.l),r(s.r){}

	auto handles() const {
		return std::tuple_cat(::handles(l),::handles(r));
	}

	auto causalCall(CausalCache& cache, const CausalStore& s) const {

		if (cache.contains(this->id) ) return cache.get<res_t>(this->id);
		else {
			cache.insert(this->id, run_ast_causal(cache,s,l)
						 + run_ast_causal(cache,s,r));
			return run_ast_causal(cache,s,l) + run_ast_causal(cache,s,r);
		}
	}
	
	auto strongCall(StrongCache& cache, const StrongStore &s) const {
		choose_strong<min_level<T,V>::value> choice{nullptr};
		return strongCall(cache,s,choice);
	}

	auto strongCall(StrongCache& cache, const StrongStore& s, std::true_type*) const {
		auto ret = run_ast_strong(cache,s,l) + run_ast_strong(cache,s,r);
		cache.insert(this->id,ret);
		return ret;
	}

	void strongCall(StrongCache &cache, const StrongStore& s, std::false_type*) const {
		run_ast_strong(cache,s,l);
		run_ast_strong(cache,s,r);
	}
	
	template<typename a, typename b>
	friend std::ostream & operator<<(std::ostream &os, const Sum<a,b>&);
};



template<unsigned long long ID, typename T, typename V>
auto find_usage(const Sum<T,V> &t){
	return choose_non_np(find_usage<ID>(t.l),find_usage<ID>(t.r));
}


//TODO: figure out why this needs to be here
template<typename T, typename V>
struct is_ConExpr<Sum<T,V> > : std::true_type {};

template<typename T, typename V>
Sum<T,V> sum(const T& t, const V& v){
	return Sum<T,V>(t,v);
}

template<Level l, typename T>
auto sum(const DummyConExpr<l>& e, const T& t){
	return t;
}

template<Level l, typename T>
auto sum(const T& t, const DummyConExpr<l>& e){
	return t;
}

template<typename T, typename V>
std::enable_if_t<is_ConExpr<T>::value, Sum<T,V> >  operator+(const T &t, const V &v){
	return sum(t,v);
}

template<typename T, typename V>
struct Equals : public ConExpr<bool, min_level<T,V>::value> {

	static_assert(is_ConExpr<T>::value,"Error: cannot equals non-expression");
	static_assert(is_ConExpr<V>::value,"Error: cannot equals non-expression");

	const int id = gensym();
	T l;
	V r;

	using res_t = decltype((std::declval<run_result<T> >()) ==
						   std::declval<run_result<V> >());
	
	Equals(const T& l, const V& r):l(l),r(r){}
	Equals(const Equals& e):l(e.l),r(e.r){}

	auto handles() const {
		return std::tuple_cat(::handles(l),::handles(r));
	}

	auto causalCall(CausalCache& cache, const CausalStore& s) const {

		if (cache.contains(this->id) ) return cache.get<res_t>(this->id);
		else {
			cache.insert(this->id, run_ast_causal(cache,s,l)
						 == run_ast_causal(cache,s,r));
			return run_ast_causal(cache,s,l) == run_ast_causal(cache,s,r);
		}
	}
	
	auto strongCall(StrongCache& cache, const StrongStore &s) const {
		choose_strong<min_level<T,V>::value> choice{nullptr};
		return strongCall(cache,s,choice);
	}

	auto strongCall(StrongCache& cache, const StrongStore& s, std::true_type*) const {
		auto ret = run_ast_strong(cache,s,l) == run_ast_strong(cache,s,r);
		cache.insert(this->id,ret);
		return ret;
	}

	void strongCall(StrongCache &cache, const StrongStore& s, std::false_type*) const {
		run_ast_strong(cache,s,l);
		run_ast_strong(cache,s,r);
	}
	
	template<typename a, typename b>
	friend std::ostream & operator<<(std::ostream &os, const Equals<a,b>&);
};

template<unsigned long long ID, typename T, typename V>
auto find_usage(const Equals<T,V> &t){
	return choose_non_np(find_usage<ID>(t.l),find_usage<ID>(t.r));
}


//TODO: figure out why this needs to be here
template<typename T, typename V>
struct is_ConExpr<Equals<T,V> > : std::true_type {};

template<typename T, typename V>
Equals<T,V> equals(const T& t, const V& v){
	return Equals<T,V>(t,v);
}

template<Level l, typename T>
auto equals(const DummyConExpr<l>& e, const T& t){
	return t;
}

template<Level l, typename T>
auto equals(const T& t, const DummyConExpr<l>& e){
	return t;
}

template<typename T, typename V>
std::enable_if_t<is_ConExpr<T>::value, Equals<T,V> >  operator==(const T &t, const V &v){
	return equals(t,v);
}


template<typename T, typename V>
struct BinaryOr : public ConExpr<bool, min_level<T,V>::value> {

	static_assert(is_ConExpr<T>::value,"Error: cannot binor non-expression");
	static_assert(is_ConExpr<V>::value,"Error: cannot binor non-expression");

	const int id = gensym();
	T l;
	V r;
	BinaryOr(const T& l, const V& r):l(l),r(r){}
	BinaryOr(const BinaryOr& b):l(b.l),r(b.r){}

	auto handles() const {
		return std::tuple_cat(::handles(l),::handles(r));
	}

	bool causalCall(CausalCache& cache, const CausalStore& s) const {

		if (cache.contains(this->id) ) return cache.get<bool>(this->id);
		else {
			cache.insert(this->id,run_ast_causal(cache,s,l) ||
						 run_ast_causal(cache,s,r));
			return run_ast_causal(cache,s,l) || run_ast_causal(cache,s,r);
		}
	}
	
	auto strongCall(StrongCache& cache, const StrongStore &s) const {
		choose_strong<min_level<T,V>::value> choice{nullptr};
		return strongCall(cache,s,choice);
	}

	bool strongCall(StrongCache& cache, const StrongStore& s, std::true_type*) const {
		auto ret = run_ast_strong(cache,s,l) || run_ast_strong(cache,s,r);
		cache.insert(this->id,ret);
		return ret;
	}

	void strongCall(StrongCache &cache, const StrongStore& s, std::false_type*) const {
		run_ast_strong(cache,s,l);
		run_ast_strong(cache,s,r);
	}
	
	template<typename a, typename b>
	friend std::ostream & operator<<(std::ostream &os, const BinaryOr<a,b>&);
};

template<unsigned long long ID, typename T, typename V>
auto find_usage(const BinaryOr<T,V> &t){
	return choose_non_np(find_usage<ID>(t.l),find_usage<ID>(t.r));
}


//TODO: figure out why this needs to be here
template<typename T, typename V>
struct is_ConExpr<BinaryOr<T,V> > : std::true_type {};

template<typename T, typename V>
BinaryOr<T,V> binor(const T& t, const V& v){
	return BinaryOr<T,V>(t,v);
}

template<Level l, typename T>
auto binor(const DummyConExpr<l>& e, const T& t){
	return t;
}

template<Level l, typename T>
auto binor(const T& t, const DummyConExpr<l>& e){
	return t;
}

template<typename T, typename V>
std::enable_if_t<is_ConExpr<T>::value, BinaryOr<T,V> >  operator||(const T &t, const V &v){
	return binor(t,v);
}

template<typename T, typename V>
struct BinaryAnd : public ConExpr<bool, min_level<T,V>::value> {

	static_assert(is_ConExpr<T>::value,"Error: cannot binand non-expression");
	static_assert(is_ConExpr<V>::value,"Error: cannot binand non-expression");

	const int id = gensym();
	T l;
	V r;
	BinaryAnd(const T& l, const V& r):l(l),r(r){}
	BinaryAnd(const BinaryAnd &ba):l(ba.l),r(ba.r){}

	auto handles() const {
		return std::tuple_cat(::handles(l),::handles(r));
	}

	bool causalCall(CausalCache& cache, const CausalStore& s) const {

		if (cache.contains(this->id) ) return cache.get<bool>(this->id);
		else {
			cache.insert(this->id,run_ast_causal(cache,s,l) &&
						 run_ast_causal(cache,s,r));
			return run_ast_causal(cache,s,l) && run_ast_causal(cache,s,r);
		}
	}
	
	auto strongCall(StrongCache& cache, const StrongStore &s) const {
		choose_strong<min_level<T,V>::value> choice{nullptr};
		return strongCall(cache,s,choice);
	}

	bool strongCall(StrongCache& cache, const StrongStore& s, std::true_type*) const {
		auto ret = run_ast_strong(cache,s,l) && run_ast_strong(cache,s,r);
		cache.insert(this->id,ret);
		return ret;
	}

	void strongCall(StrongCache &cache, const StrongStore& s, std::false_type*) const {
		run_ast_strong(cache,s,l);
		run_ast_strong(cache,s,r);
	}
	
	template<typename a, typename b>
	friend std::ostream & operator<<(std::ostream &os, const BinaryAnd<a,b>&);
};

template<unsigned long long ID, typename T, typename V>
auto find_usage(const BinaryAnd<T,V> &t){
	return choose_non_np(find_usage<ID>(t.l),find_usage<ID>(t.r));
}


//TODO: figure out why this needs to be here
template<typename T, typename V>
struct is_ConExpr<BinaryAnd<T,V> > : std::true_type {};

template<typename T, typename V>
BinaryAnd<T,V> binand(const T& t, const V& v){
	return BinaryAnd<T,V>(t,v);
}

template<Level l, typename T>
auto binand(const DummyConExpr<l>& e, const T& t){
	return t;
}

template<Level l, typename T>
auto binand(const T& t, const DummyConExpr<l>& e){
	return t;
}

template<typename T, typename V>
std::enable_if_t<is_ConExpr<T>::value, BinaryAnd<T,V> >  operator&&(const T &t, const V &v){
	return binand(t,v);
}


template<typename T>
struct Not : public ConExpr<bool, get_level<T>::value> {

	static_assert(is_ConExpr<T>::value,"Error: cannot negate non-expression");

	const int id = gensym();
	T v;
	Not(const T& t):v(t){}
	Not(const Not& n):v(n.v){}

	auto handles() const {
		return v.handles();
	}

	bool causalCall(CausalCache& cache, const CausalStore& s) const {

		if (cache.contains(this->id) ) return cache.get<bool>(this->id);
		else {
			cache.insert(this->id,!v.causalCall(cache,s));
			return !v.causalCall(cache,s);
		}
	}
	
	auto strongCall(StrongCache& cache, const StrongStore &s) const {
		choose_strong<get_level<T>::value> choice{nullptr};
		return strongCall(cache,s,choice);
	}

	bool strongCall(StrongCache& cache, const StrongStore& s, std::true_type*) const {
		bool ret = !v.strongCall(cache,s);
		cache.insert(this->id,ret);
		return ret;
	}

	void strongCall(StrongCache &cache, const StrongStore& s, std::false_type*) const {
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
	IsValid(const IsValid &i):t(i.t){}

	auto handles() const {
		return ::handles(t);
	}

	bool causalCall(CausalCache& cache, const CausalStore& s) const {
		if (cache.contains(this->id) ) return cache.get<bool>(this->id);
		else {
			auto prev = context::current_context(cache);
			context::set_context(cache,context::t::validity);
			bool ret = run_ast_causal(cache,s,t).isValid();
			context::set_context(cache,prev);
			cache.insert(this->id,ret);
			return ret;
		}
	}
	
	auto strongCall(StrongCache& cache, const StrongStore &s) const {
		choose_strong<get_level<T>::value> choice{nullptr};
		return strongCall(cache,s,choice);
	}

	bool strongCall(StrongCache& cache, const StrongStore&s, std::true_type*) const {
		auto prev = context::current_context(cache);
		context::set_context(cache,context::t::validity);
		bool ret = run_ast_strong(cache,s,t).isValid();
		context::set_context(cache,prev);
		cache.insert(this->id,ret);
		return ret;
	}

	void strongCall(StrongCache &cache, const StrongStore& s, std::false_type*) const {
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
