#pragma once
#include "ConExpr.hpp"
#include <string>

struct GeneralTemp {};

//the level here is for referencing the temporary later.
//it's the endorsement check!
template<Level l, typename T>
struct Temporary : public GeneralTemp, public ConStatement<get_level<T>::value> {
	static_assert(is_ConExpr<T>::value,
				  "Error: can only assign temporary the result of expressions");
	static_assert(l == Level::causal ||
				  get_level<T>::value == Level::strong,
				  "Error: flow violation");
public:
	const int id;
	const T t;
	Temporary(const T& t):id(gensym()),t(t){}
	Temporary(const int id, const T& t):id(id),t(t){}
	
	auto getReadSet() const {
		return t.getReadSet();
	}
	
	bool operator()(Store &s) const {
		typedef typename std::decay<decltype(t(s))>::type R;
		if (!s.contains(id)) s[id].reset((Store::stored) new R(t(s)));
		return true;
	}
	template<Level l2, typename i2>
	friend std::ostream & operator<<(std::ostream &os, const Temporary<l2,i2>&);
};	
template<Level l2, typename i2>
std::ostream & operator<<(std::ostream &os, const Temporary<l2,i2>& t){
	return os << "__x" << t.id << "<" << levelStr<l2>() << ">" <<  " = " << t.t;
}

template<typename T>
struct TemporaryMutation : public ConStatement<get_level<T>::value> {
	const std::string name;
	const int id;
	const T t;

	TemporaryMutation(const std::string &name, int id, const T& t)
		:name(name),id(id),t(t) {}
	
	bool operator()(Store &s) const {
		typedef typename std::decay<decltype(run_ast(s,t))>::type R;
		s[id].reset((Store::stored) new R(run_ast(s,t)));
		return true;
	}

	auto getReadSet() const {
		return get_ReadSet(t);
	}
	
};

template<typename T>
std::ostream & operator<<(std::ostream &os, const TemporaryMutation<T>& t){
	return os << t.name << " := " << t.t;
}

template<unsigned long long ID, Level l, typename T>
struct MutableTemporary : public Temporary<l,T> {
	const std::string &name;
	MutableTemporary(const std::string& name, const T& t):
		Temporary<l,T>(std::hash<std::string>()(name),t),
		name(name){}

	bool operator()(Store &s) const {
		typedef typename std::decay<decltype(this->t(s))>::type R;
		s[this->id].reset((Store::stored) new R(this->t(s)));
		return true;
	}

	typedef typename std::integral_constant<Level,l>::type level;
	typedef T type;
	typedef std::true_type found;
	typedef typename std::integral_constant<unsigned long long, ID>::type key;
	
};


template<unsigned long long id, Level l2, typename i2>
std::ostream & operator<<(std::ostream &os, const MutableTemporary<id,l2,i2>& t){
	return os << t.name << "<" << levelStr<l2>() << ">" <<  " = " << t.t;
}


template<Level l, typename T>
class CSConstant;


#define MutAssigner(c) MutCreator<unique_id<(sizeof(c) / sizeof(char)) - 1>(c)>{c}
#define ImmutAssigner(c) ImmutCreator<unique_id<(sizeof(c) / sizeof(char)) - 1>(c)>{c}


//todo: now it looks like we know how to stick the identifier in the type.
//we still need to figure out how to retrieve the type and level when referencing
//the variable later in the program.


template<Level l, typename T, typename Temp>
struct RefTemporary : public ConExpr<decltype(run_ast(mke_store(), *mke_p<T>())),l> {
	const Temp t;
	const std::string name;
	RefTemporary(const Temporary<l,T> &t):t(t),name(std::string("__x") + std::to_string(t.id))
		{static_assert(std::is_same<Temporary<l,T>, Temp>::value,
					   "Error: wrong constructor used for RT");}
	
	template<unsigned long long id>
	RefTemporary(const MutableTemporary<id,l,T> &t):t(t),name(t.name)
		{static_assert(std::is_same<MutableTemporary<id,l,T>, Temp>::value,
					   "Error: wrong constructor used for RT");}

	auto getReadSet() const {
		return t.getReadSet();
	}
	decltype(run_ast(mke_store(),*mke_p<T>())) operator()(const Store &s) const{
		return call(s,t);
	}

	template<typename E>
	enable_if<!std::is_same<Temporary<l,T>, Temp>::value, TemporaryMutation<E> >
	operator=(const E &e) const {
		static_assert(is_ConExpr<E>::value,"Error: attempt to assign non-Expr");
		TemporaryMutation<E> r{name,t.id,e};
		return r;
	}

	template<typename E>
	enable_if<std::is_same<Temporary<l,T>, Temp>::value, TemporaryMutation<E> >
	operator=(const E &e) const {
		static_assert(is_ConExpr<E>::value,"Error: attempt to assign non-Expr");
		static_assert(!is_ConExpr<E>::value,"Error: attempt to mutate immutable temporary.");
		TemporaryMutation<E> r{name,t.id,e};
		return r;
	}



private:
	static auto call(const Store &s, const Temporary<l,T> &t){
		typedef decltype(run_ast(mke_store(),t.t)) R;
		return *((R*) s.at(t.id).get());
	}

	static_assert(!is_ConStatement<decltype(
					  call(mke_store(),*mke_p<Temporary<l,T> >())
					  )>::value);
};


template<unsigned long long ID>
struct MutCreator {
	const std::string &name;

	template<typename T>
	auto operator=(const T& t) const {
		static_assert(is_ConExpr<T>::value, "Error: cannot assign non-expression");
		static constexpr Level l = get_level<T>::value;
		RefTemporary<l,T,MutableTemporary<ID,l,T > >
			rt(MutableTemporary<ID,l,T >(name,t));
		return rt;
	}
};

template<unsigned long long ID>
struct ImmutCreator {
	const std::string &name;

	template<typename T>
	auto operator=(const T& t) const {
		static_assert(is_ConExpr<T>::value, "Error: cannot assign non-expression");
		static constexpr Level l = get_level<T>::value;
		RefTemporary<l,T,Temporary<l,T > >
			rt(Temporary<l,T >(std::hash<std::string>()(name),t));
		return rt;
	}

};


//TODO: figure out why this needs to be here
template<Level l, typename T, typename E>
struct is_ConExpr<RefTemporary<l,T, E> > : std::true_type {};


template<Level l2, typename T2, typename E>
std::ostream & operator<<(std::ostream &os, const RefTemporary<l2,T2, E>& t){
	return os << t.name <<  "<" << levelStr<l2>() << ">";
}


struct nope{
	typedef std::false_type found;
};

std::ostream & operator<<(std::ostream &os, const nope& ){
	return os << "nope!";
}

template<Level l, typename T, typename E>
bool is_reftemp(const RefTemporary<l,T, E> *){
	return true;
}

template<typename T>
struct is_RefTemporary : std::integral_constant<bool,is_reftemp(mke_p<T>())>::type
{};
