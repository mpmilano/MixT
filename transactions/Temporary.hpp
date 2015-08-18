#pragma once
#include "ConExpr.hpp"
#include <string>

struct GeneralTemp {
	const std::string name;
	const std::string gets;
	GeneralTemp(const std::string &n, const std::string &g):name(n),gets(g) {}
};

//the level here is for referencing the temporary later.
//it's the endorsement check!
template<unsigned long long ID, Level l, typename T>
struct Temporary : public GeneralTemp, public ConStatement<get_level<T>::value> {
	static_assert(is_ConExpr<T>::value,
				  "Error: can only assign temporary the result of expressions");
	static_assert(l == Level::causal ||
				  get_level<T>::value == Level::strong,
				  "Error: flow violation");

	const T t;
	const int id;
	Temporary(const T& t):GeneralTemp(std::to_string(gensym()),std::to_string(t)),t(t),id(std::hash<std::string>()(this->name)){}
	Temporary(const std::string name, const T& t):GeneralTemp(name,std::to_string(t)),t(t),id(std::hash<std::string>()(name)){}
	
	auto getReadSet() const {
		return t.getReadSet();
	}

	auto strongCall(Store &c, Store &s) const {
		std::integral_constant<bool,get_level<T>::value==Level::strong>* choice = nullptr;
		return strongCall(c,s,choice);
	}

	auto strongCall(Store &c, Store &s, std::true_type*) const {
		typedef typename std::decay<decltype(t.strongCall(c,s))>::type R;
		if (!s.contains(id)) s[id].reset((Store::stored) new R(t.strongCall(c,s)));
		return true;
	}

	void strongCall(Store &c, const Store &s, std::false_type*) const {
		t.strongCall(c,s);
	}

	auto causalCall(Store &c, Store &s) const {
		std::integral_constant<bool,get_level<T>::value==Level::causal>* choice = nullptr;
		return causalCall(c,s,choice);
	}

	auto causalCall(Store &c, Store &s,std::true_type*) const {
		typedef typename std::decay<decltype(t.causalCall(c,s))>::type R;
		if (!s.contains(id)) s[id].reset((Store::stored) new R(t.causalCall(c,s)));
		return true;
	}

	auto causalCall(Store &c, Store &s,std::false_type*) const {
		//noop.  We've already executed this instruction.
		return true;
	}

};

template<unsigned long long ID, Level l, typename T, typename Temp>
auto find_usage(const Temporary<ID,l,T> &rt){
	return heap_copy(rt);
}

template<unsigned long long ID, unsigned long long ID2, Level l, typename T>
struct contains_temporary<ID, Temporary<ID2,l,T> > : std::integral_constant<bool, ID == ID2> {};

template<Level l2, typename i2, unsigned long long id>
std::ostream & operator<<(std::ostream &os, const Temporary<id,l2,i2>& t){
	return os << t.name << "<" << levelStr<l2>() << ">" <<  " = " << t.t;
}

template<typename T>
struct TemporaryMutation : public ConStatement<get_level<T>::value> {
	const std::string name;
	const int id;
	const T t;

	TemporaryMutation(const std::string &name, int id, const T& t)
		:name(name),id(id),t(t) {}

	auto strongCall(Store &c, Store &s) const {
		std::integral_constant<bool,get_level<T>::value==Level::strong>* choice = nullptr;
		return strongCall(c,s,choice);
	}

	auto strongCall(Store &c, Store &s, std::true_type*) const {
		typedef typename std::decay<decltype(run_ast_strong(c,s,t))>::type R;
		s[id].reset((Store::stored) new R(run_ast_strong(c,s,t)));
		return true;
	}

	void strongCall(Store &c, const Store &s, std::false_type*) const {
		t.strongCall(c,s);
	}

	auto causalCall(Store &c, Store &s) const {
		std::integral_constant<bool,get_level<T>::value==Level::causal>* choice = nullptr;
		return causalCall(c,s,choice);
	}

	auto causalCall(Store &c, Store &s,std::true_type*) const {
		typedef typename std::decay<decltype(run_ast_causal(c,s,t))>::type R;
		s[id].reset((Store::stored) new R(run_ast_causal(c,s,t)));
		return true;
	}

	auto causalCall(Store &c, Store &s,std::false_type*) const {
		//noop.  We've already executed this instruction.
		return true;
	}
	
	auto getReadSet() const {
		return get_ReadSet(t);
	}
	
};


template<unsigned long long ID, typename T>
struct contains_temporary<ID, TemporaryMutation<T> > : contains_temporary<ID,T> {};


template<unsigned long long ID, typename T>
auto find_usage(const TemporaryMutation<T> &t){
	return find_usage<ID>(t.t);
}

template<typename T>
std::ostream & operator<<(std::ostream &os, const TemporaryMutation<T>& t){
	return os << t.name << " := " << t.t;
}

template<unsigned long long ID, Level l, typename T>
struct MutableTemporary : public Temporary<ID, l,T> {
	MutableTemporary(const std::string& name, const T& t):
		Temporary<ID,l,T>(name,t){}

	typedef typename std::integral_constant<Level,l>::type level;
	typedef T type;
	typedef std::true_type found;
	typedef typename std::integral_constant<unsigned long long, ID>::type key;


};

template<unsigned long long ID, Level l, typename T>
auto find_usage(const MutableTemporary<ID,l,T> &rt){
	return heap_copy(rt);
}

template<unsigned long long ID, Level l, typename T>
struct contains_temporary<ID, MutableTemporary<ID,l,T> > : std::true_type {};

template<unsigned long long ID, unsigned long long ID2, Level l, typename T>
struct contains_temporary<ID, MutableTemporary<ID2,l,T> > : std::integral_constant<bool, ID == ID2> {};


template<unsigned long long ID, unsigned long long ID2, Level l, typename T>
enable_if<ID != ID2,std::nullptr_t> find_usage(const MutableTemporary<ID2,l,T> &rt){
	return nullptr;
}

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


template<unsigned long long id, Level l, typename T, typename Temp>
struct RefTemporary : public ConExpr<decltype(run_ast(mke_store(), *mke_p<T>())),l> {
	const Temp t;
	const std::string name;

	//Note: this ID will change
	//every time we copy this class.
	//every copy should have a unique ID.
	const int local_id = gensym();
	RefTemporary(const Temporary<id,l,T> &t):t(t),name(std::string("__x") + std::to_string(t.id))
		{static_assert(std::is_same<Temporary<id,l,T>, Temp>::value,
					   "Error: wrong constructor used for RT");}

	RefTemporary(const MutableTemporary<id,l,T> &t):t(t),name(t.name)
		{static_assert(std::is_same<MutableTemporary<id,l,T>, Temp>::value,
					   "Error: wrong constructor used for RT");}

	RefTemporary(const RefTemporary& rt):t(rt.t),name(rt.name),local_id(gensym()){}

	auto getReadSet() const {
		return t.getReadSet();
	}
	decltype(run_ast(mke_store(),*mke_p<T>())) operator()(const Store &s) const{
		return call(s,t);
	}

	template<typename E>
	enable_if<!std::is_same<Temporary<id,l,T>, Temp>::value, TemporaryMutation<E> >
	operator=(const E &e) const {
		static_assert(is_ConExpr<E>::value,"Error: attempt to assign non-Expr");
		TemporaryMutation<E> r{name,t.id,e};
		return r;
	}

	template<typename E>
	enable_if<std::is_same<Temporary<id,l,T>, Temp>::value, TemporaryMutation<E> >
	operator=(const E &e) const {
		static_assert(is_ConExpr<E>::value,"Error: attempt to assign non-Expr");
		static_assert(!is_ConExpr<E>::value,"Error: attempt to mutate immutable temporary.");
		TemporaryMutation<E> r{name,t.id,e};
		return r;
	}



private:
	static auto call(const Store &s, const Temporary<id,l,T> &t){
		typedef decltype(run_ast(mke_store(),t.t)) R;
		return *((R*) s.at(t.id).get());
	}

	static_assert(!is_ConStatement<decltype(
					  call(mke_store(),*mke_p<Temporary<id,l,T> >())
					  )>::value);
};

template<unsigned long long ID, Level l, typename T, typename Temp>
auto find_usage(const RefTemporary<ID,l,T,Temp> &rt){
	return heap_copy(rt.t);
}

template<unsigned long long ID, unsigned long long ID2, Level l, typename T, typename Temp>
enable_if<ID != ID2, std::nullptr_t> find_usage(const RefTemporary<ID,l,T,Temp> &rt){
	return nullptr;
}

template<unsigned long long ID, unsigned long long ID2, Level l, typename T, typename Temp>
struct contains_temporary<ID, RefTemporary<ID2,l,T,Temp> > : std::integral_constant<bool, ID == ID2> {};


template<unsigned long long ID>
struct MutCreator {
	const std::string &name;

	template<typename T>
	auto operator=(const T& t) const {
		static_assert(is_ConExpr<T>::value, "Error: cannot assign non-expression");
		static constexpr Level l = get_level<T>::value;
		RefTemporary<ID,l,T,MutableTemporary<ID,l,T > >
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
		RefTemporary<ID,l,T,Temporary<ID,l,T > >
			rt(Temporary<ID,l,T >(std::hash<std::string>()(name),t));
		return rt;
	}

};


//TODO: figure out why this needs to be here
template<Level l, typename T, typename E, unsigned long long id>
struct is_ConExpr<RefTemporary<id,l,T, E> > : std::true_type {};


template<Level l2, typename T2, typename E, unsigned long long id>
std::ostream & operator<<(std::ostream &os, const RefTemporary<id,l2,T2, E>& t){
	return os << t.name <<  "<" << levelStr<l2>() << ">";
}


struct nope{
	typedef std::false_type found;
};

std::ostream & operator<<(std::ostream &os, const nope& ){
	return os << "nope!";
}

template<Level l, typename T, typename E, unsigned long long id>
bool is_reftemp(const RefTemporary<id,l,T, E> *){
	return true;
}

template<typename T>
struct is_RefTemporary : std::integral_constant<bool,is_reftemp(mke_p<T>())>::type
{};
