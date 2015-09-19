#pragma once
#include "ConExpr.hpp"
#include "CommonExprs.hpp"
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
	
	static_assert(can_flow(get_level<T>::value,l),"Error: flow violation");

	const T t;
	const int store_id;
	Temporary(const std::string name, const T& t):GeneralTemp(name,to_string(t)),t(t),store_id(std::hash<std::string>()(name)){
		static_assert(get_level<Temporary>::value == get_level<T>::value,"error: you overrode get_level wrong for Temporaries");
	}

	auto handles() const {
		return ::handles(t);
	}

	auto strongCall(StrongCache& c, StrongStore &s) const {

		choose_strong<get_level<T>::value > choice{nullptr};
		return strongCall(c,s,choice);
	}

	auto strongCall(StrongCache& c, StrongStore &s, std::true_type*) const {
		typedef typename std::decay<decltype(run_ast_strong(c,s,t))>::type R;
		s.emplace<R>(store_id, run_ast_strong(c,s,t));
		return true;
	}

	void strongCall(StrongCache& c, const StrongStore &s, std::false_type*) const {
		run_ast_strong(c,s,t);
	}

	auto causalCall(CausalCache& c, CausalStore &s) const {
		choose_causal<get_level<T>::value > choice{nullptr};
		return causalCall(c,s,choice);
	}

	auto causalCall(CausalCache& c, CausalStore &s,std::true_type*) const {
		typedef typename std::decay<decltype(t.causalCall(c,s))>::type R;
		s.emplace<R>(store_id,run_ast_causal(c,s,t));
		return true;
	}

	auto causalCall(CausalCache& c, CausalStore &s,std::false_type*) const {
		//noop.  We've already executed this instruction.
		return true;
	}

};

template<unsigned long long ID, Level l, typename Temp>
struct chld_min_level<Temporary<ID,l,Temp> > : level_constant<l> {};

template<unsigned long long ID, Level l, typename Temp>
struct chld_max_level<Temporary<ID,l,Temp> > : level_constant<l> {};

template<unsigned long long ID, Level l, typename T, typename Temp>
auto find_usage(const Temporary<ID,l,T> &rt){
	return shared_copy(rt);
}

template<unsigned long long ID, unsigned long long ID2, Level l, typename T>
struct contains_temporary<ID, Temporary<ID2,l,T> > : std::integral_constant<bool, ID == ID2> {};

template<typename T>
struct TemporaryMutation : public ConStatement<get_level<T>::value> {
	const std::string name;
	const int store_id;
	const T t;

	TemporaryMutation(const std::string &name, int id, const T& t)
		:name(name),store_id(id),t(t) {}

	auto handles() const {
		return ::handles(t);
	}
	
	auto strongCall(StrongCache& c, StrongStore &s) const {
		choose_strong<get_level<T>::value > choice{nullptr};
		return strongCall(c,s,choice);
	}

	auto strongCall(StrongCache& c, StrongStore &s, std::true_type*) const {
		typedef typename std::decay<decltype(run_ast_strong(c,s,t))>::type R;
		s.emplace_ovrt<R>(store_id,run_ast_strong(c,s,t));
		return true;
	}

	void strongCall(StrongCache& c, const StrongStore &s, std::false_type*) const {
		t.strongCall(c,s);
	}

	auto causalCall(CausalCache& c, CausalStore &s) const {
		choose_causal<get_level<T>::value > choice{nullptr};
		return causalCall(c,s,choice);
	}

	auto causalCall(CausalCache& c, CausalStore &s,std::true_type*) const {
		typedef typename std::decay<decltype(run_ast_causal(c,s,t))>::type R;
		s.emplace_ovrt<R>(store_id,run_ast_causal(c,s,t));
		return true;
	}

	auto causalCall(CausalCache& c, CausalStore &s,std::false_type*) const {
		//noop.  We've already executed this instruction.
		return true;
	}
	
};


template<typename T>
struct chld_min_level<TemporaryMutation<T> > : chld_min_level<T> {};
template<typename T>
struct chld_min_level<const TemporaryMutation<T> > : chld_min_level<T> {};


template<typename T>
struct chld_max_level<TemporaryMutation<T> > : chld_min_level<T> {};
template<typename T>
struct chld_max_level<const TemporaryMutation<T> > : chld_min_level<T> {};


template<unsigned long long ID, typename T>
struct contains_temporary<ID, TemporaryMutation<T> > : contains_temporary<ID,T> {};


template<unsigned long long ID, typename T>
auto find_usage(const TemporaryMutation<T> &t){
	return find_usage<ID>(t.t);
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
	return shared_copy(rt);
}

template<unsigned long long ID, unsigned long long ID2, Level l, typename T>
struct contains_temporary<ID, MutableTemporary<ID2,l,T> > : std::integral_constant<bool, ID == ID2> {};


template<unsigned long long ID, unsigned long long ID2, Level l, typename T>
std::enable_if_t<ID != ID2,std::nullptr_t> find_usage(const MutableTemporary<ID2,l,T> &rt){
	return nullptr;
}

template<Level l, typename T>
class CSConstant;


#define MutAssigner(c) MutCreator<unique_id<(sizeof(c) / sizeof(char)) - 1>(c)>{c}
#define ImmutAssigner(c) ImmutCreator<unique_id<(sizeof(c) / sizeof(char)) - 1>(c)>{c}

namespace{
	bool debug_forbid_copy = false;
}

template<unsigned long long ID, Level l, typename T, typename Temp>
struct RefTemporary : public ConExpr<run_result<T>,l> {
private:
	RefTemporary(const Temp& t, const std::string& name, int id):t(t),name(name),id(id){}
public:
	
	const Temp t;
	const std::string name;

	//Note: this ID will change
	//every time we copy this class.
	//every copy should have a unique ID.
	const int id = gensym();

	RefTemporary(const Temp &t):t(t),name(t.name) {}

	RefTemporary(const RefTemporary& rt):t(rt.t),name(rt.name){
		assert(!debug_forbid_copy);
	}
	
	RefTemporary(RefTemporary&& rt):t(rt.t),name(rt.name),id(rt.id){}

	RefTemporary clone() const {
		return RefTemporary(t,name,id);
	}

	auto handles() const {
		return t.handles();
	}

	auto strongCall(StrongCache& cache, const StrongStore &s) const {
		//TODO - endorsements should happen somewhere around here, right?
		//todo: dID I want the level of the expression which assigned the temporary?
		
		choose_strong<get_level<Temp>::value > choice{nullptr};
		try {
			auto ret = strongCall(cache, s,choice);
			std::cout << "refTemping this value (" << t.store_id << "): " << ret << std::endl;
			return ret;
		}
		catch (const StoreMiss&){
			std::cerr << "tried to reference variable " << name << std::endl;
			assert(false && "we don't have that in the store");
		}
	}

	auto strongCall(StrongCache& cache, const StrongStore &s, std::true_type*) const {
		//std::cout << "inserting RefTemp " << name << " (" << id<< ") into cache "
		//		  << &cache << std::endl;
		auto ret = call<StoreType::StrongStore>(s, t);
		cache.insert(id,ret);
		//std::cout << "RefTemp ID " << this->id << " inserting into Cache " << &cache << " value: " << ret << std::endl;
		return ret;
	}

	void strongCall(StrongCache& cache, const StrongStore &s, std::false_type*) const {
		//we haven't even done the assignment yet. nothing to see here.
	}

	auto causalCall(CausalCache& cache, const CausalStore &s) const {
		
		typedef decltype(call<StoreType::CausalStore>(s,t)) R;
		if (cache.contains(this->id)) {
			return cache.get<R>(this->id);
		}
		else {
			try {
				return call<StoreType::CausalStore>(s,t);
			}
			catch (const StoreMiss&){
				std::cerr << "Couldn't find this in the store: " << name << std::endl;
				assert(false && "Not in the store");
			}
		}
	}

	template<typename E>
	std::enable_if_t<!std::is_same<Temporary<ID,l,T>, Temp>::value, TemporaryMutation<decltype(wrap_constants(*mke_p<E>()))> >
	operator=(const E &e) const {
		static_assert(is_ConExpr<E>::value,"Error: attempt to assign non-Expr");
		auto wrapped = wrap_constants(e);
		TemporaryMutation<decltype(wrapped)> r{name,t.store_id,wrapped};
		return r;
	}

	template<typename E>
	std::enable_if_t<std::is_same<Temporary<ID,l,T>, Temp>::value, TemporaryMutation<E> >
	operator=(const E &e) const {
		static_assert(is_ConExpr<E>::value,"Error: attempt to assign non-Expr");
		static_assert(!is_ConExpr<E>::value,"Error: attempt to mutate immutable temporary.");
		TemporaryMutation<E> r{name,t.store_id,wrap_constants(e)};
		return r;
	}



private:
	template<StoreType st, restrict(is_store(st))>
	static auto call(const StoreMap<st> &s, const Temporary<ID,l,T> &t) ->
		run_result<decltype(t.t)>
		{
			typedef run_result<decltype(t.t)> R;
			static_assert(neg_error_helper<is_ConStatement,R>::value,"Static assert failed");
			return s. template get<R>(t.store_id);
		}
};

template<unsigned long long ID, Level l, typename T, typename Temp>
auto find_usage(const RefTemporary<ID,l,T,Temp> &rt){
	return shared_copy(rt.t);
}

template<unsigned long long ID, unsigned long long ID2, Level l, typename T, typename Temp>
std::enable_if_t<ID != ID2, std::nullptr_t> find_usage(const RefTemporary<ID2,l,T,Temp> &rt){
	return nullptr;
}

template<unsigned long long ID, unsigned long long ID2, Level l, typename T, typename Temp>
struct contains_temporary<ID, RefTemporary<ID2,l,T,Temp> > : std::integral_constant<bool, ID == ID2> {};


template<unsigned long long ID>
struct MutCreator {
	const std::string &name;

	template<typename T>
	auto operator=(const T& t_) const {
		static_assert(is_ConExpr<T>::value, "Error: cannot assign non-expression");
		auto t = wrap_constants(t_);
		static constexpr Level l = get_level<T>::value;
		RefTemporary<ID,l,decltype(t),MutableTemporary<ID,l,decltype(t) > >
			rt(MutableTemporary<ID,l,decltype(t) >(name,t));
		return rt;
	}
};

template<unsigned long long ID>
struct ImmutCreator {
	const std::string &name;

	template<typename T>
	auto operator=(const T& t_) const {
		static_assert(is_ConExpr<T>::value, "Error: cannot assign non-expression");
		auto t = wrap_constants(t_);
		static constexpr Level l = get_level<T>::value;
		RefTemporary<ID,l,decltype(t),Temporary<ID,l,decltype(t) > >
			rt(Temporary<ID,l,decltype(t) >(name,t));
		return rt;
	}

};


//TODO: figure out why this needs to be here
template<Level l, typename T, typename E, unsigned long long id>
struct is_ConExpr<RefTemporary<id,l,T, E> > : std::true_type {};


struct nope{
	typedef std::false_type found;
};

template<Level l, typename T, typename E, unsigned long long id>
bool is_reftemp(const RefTemporary<id,l,T, E> *){
	return true;
}

template<typename T>
struct is_RefTemporary : std::integral_constant<bool,is_reftemp(mke_p<T>())>::type
{};
