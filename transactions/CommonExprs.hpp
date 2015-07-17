#pragma once
#include "ConExpr.hpp"
#include "Temporary.hpp"
#include "../BitSet.hpp"

template<Level l, int i>
class CSInt : public ConExpr<l>, public std::integral_constant<int,i>::type {
public:
	CSInt(){}

	BitSet<backend::HandleAbbrev> getReadSet() const {
		return BitSet<backend::HandleAbbrev>();
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
constexpr bool is_base_CS_f(const CSInt<l,i>* ){
	return true;
}

template<typename T>
struct Not : public ConExpr<get_level<T>::value> {

	static_assert(is_ConExpr<T>::value,"Error: cannot negate non-expression");
	
	T v;
	Not(const T& t):v(t){}
	
	auto operator()(Store &s) const {
		return !v(s);
	}

	BitSet<backend::HandleAbbrev> getReadSet() const {
		return v.getReadSet();
	}

	template<typename i2>
	friend std::ostream & operator<<(std::ostream &os, const Not<i2>&);
};

template<typename i2>
std::ostream & operator<<(std::ostream &os, const Not<i2>& n){
	return os << "!" << n.v;
}


template<typename T>
Not<T> make_not(const T& t){
	return Not<T>(t);
}

template<backend::Level l>
DummyConExpr<l> make_not(const DummyConExpr<l>& e){
	return e;
}

template<typename T>
struct IsValid : public ConExpr<get_level<T>::value> {
	static_assert(backend::is_handle<T>::value,"error: IsValid designed for referential integrity of handles.");

	const T t;
	
	IsValid(const T &t):t(t){}
	
	bool operator()(const Store &) const {
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

	template<typename T2>
	friend std::ostream & operator<<(std::ostream &os, const IsValid<T2>&);
};

template<typename T2>
std::ostream & operator<<(std::ostream &os, const IsValid<T2> &t){
	backend::HandleAbbrev ha = t.t;
	BitSet<backend::HandleAbbrev>::member_t pr = ha;
	unsigned long long pr2 = pr;
	return os << "isValid(" << pr2 << ")";
}

template<backend::Level l, typename T>
struct RefTemporary : public ConExpr<l> {
	const Temporary<l,T> t;
	RefTemporary(const Temporary<l,T> &t):t(t){}

	auto getReadSet() const {
		return t.getReadSet();
	}
	auto operator()(const Store &s) const{
		return call(s,t);
	}

private:
	static auto call(const Store &s, const Temporary<l,T> &t){
		typedef decltype(t.t(*mke_p<Store>())) R;
		return *((R*) s.at(t.id).get());
	}

	static_assert(!is_ConStatement<decltype(
					  call(*mke_p<Store>(),*mke_p<Temporary<l,T> >())
					  )>::value);
	template<backend::Level l2, typename T2>
	friend std::ostream & operator<<(std::ostream &os, const RefTemporary<l2,T2>&);
};

template<backend::Level l2, typename T2>
std::ostream & operator<<(std::ostream &os, const RefTemporary<l2,T2>& t){
	return os << "RefTemp: " << t.t;
}

template<backend::Level l, typename T>
auto ref_temp(const Temporary<l,T> &t){
	return RefTemporary<l,T>(t);
}

template<backend::Level l>
auto ref_temp(const DummyConExpr<l> &r){
	return r;
}

template<backend::Level l, typename T>
bool is_reftemp(const RefTemporary<l,T> *){
	return true;
}

template<typename T>
struct is_RefTemporary : std::integral_constant<bool,is_reftemp(mke_p<T>())>::type
{};

template<typename T>
auto isValid(const T &t){
	return IsValid<T>(t);
}
	
template<typename T, typename... Handles>
struct FreeExpr : public ConExpr<min_level<Handles...>::value > {

	//todo: idea here is that only read-only things can be done to the handles
	//in this context.  Try to make that a reality please.
	std::unique_ptr<std::function<T ()> > f;
	std::unique_ptr<const BitSet<backend::HandleAbbrev> > rs;
	
	FreeExpr(int,
			 std::function<T (const typename backend::extract_type<Handles>::type & ... )> f,
			 Handles... h)
		:f(new std::function<T ()>([&,f,h...](){return f(h.get()...);})),
		 rs(new BitSet<backend::HandleAbbrev>(setify(h.abbrev()...)))
		{}

	FreeExpr(const FreeExpr&) = delete;

	T operator()(Store &) const {
		return (*f)();
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


