#pragma once
#include "ConExpr.hpp"
#include <string>
	
//the level here is for referencing the temporary later.
//it's the endorsement check!
	template<Level l, typename T>
	struct Temporary : public ConStatement<get_level<T>::value> {
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

template<Level l, typename T>
auto make_temp(const T& t){
	return Temporary<l,T>(t);
}

template<Level, Level l>
auto make_temp(const DummyConExpr<l>& r){
	return r;
}
template<Level l2, typename i2>
std::ostream & operator<<(std::ostream &os, const Temporary<l2,i2>& t){
	return os << "__x" << t.id << "<" << levelStr<l2>() << ">" <<  " = " << t.t;
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
	
	CONNECTOR_OP;
};

template<unsigned long long id, Level l, typename T>
auto make_mut_temp(const std::string& name, const T& t){
	return MutableTemporary<id,l,T>(name,t);
}

template<unsigned long long, Level, Level l>
auto make_mut_temp(const std::string& , const DummyConExpr<l>& r){
	return r;
}

template<unsigned long long id, Level l2, typename i2>
std::ostream & operator<<(std::ostream &os, const MutableTemporary<id,l2,i2>& t){
	return os << t.name << "<" << levelStr<l2>() << ">" <<  " = " << t.t;
}

template<Level l, typename T>
class CSConstant;

#define temp(a,b,c) _temp<a,b,decltype(c),unique_id<(sizeof(c) / sizeof(char)) - 1>(c)>(c)


//todo: now it looks like we know how to stick the identifier in the type.
//we still need to figure out how to retrieve the type and level when referencing
//the variable later in the program.

template<Level l, typename T, typename Str, unsigned long long id>
auto _temp(const Str &str){
	struct unassigned_temp {
		const std::string s;
		MutableTemporary<id,l,CSConstant<l,T> > operator=(const T& t){
			return make_mut_temp<id,l,CSConstant<l,T> >(s, CSConstant<l,T>(t));
		}
	} r{str};
	return r;
}

template<Level l, typename T>
struct RefTemporary : public ConExpr<decltype(mke<T>()(*mke_p<Store>())),l> {
	const Temporary<l,T> t;
	const std::string name;
	RefTemporary(const Temporary<l,T> &t):t(t),name(std::string("__x") + std::to_string(t.id)){}
	template<unsigned long long id>
	RefTemporary(const MutableTemporary<id,l,T> &t):t(t),name(t.name){}

	auto getReadSet() const {
		return t.getReadSet();
	}
	decltype(mke<T>()(*mke_p<Store>())) operator()(Store &s) const{
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
	template<Level l2, typename T2>
	friend std::ostream & operator<<(std::ostream &os, const RefTemporary<l2,T2>&);
};

//TODO: figure out why this needs to be here
template<Level l, typename T>
struct is_ConExpr<RefTemporary<l,T> > : std::true_type {};


template<Level l2, typename T2>
std::ostream & operator<<(std::ostream &os, const RefTemporary<l2,T2>& t){
	return os << t.name <<  "<" << levelStr<l2>() << ">";
}

template<Level l, typename T>
auto ref_temp(const Temporary<l,T> &t){
	return RefTemporary<l,T>(t);
}

template<Level l>
auto ref_temp(const DummyConExpr<l> &r){
	return r;
}

template<unsigned long long id>
struct refstr{
};

#define ref(c) refstr<unique_id<(sizeof(c) / sizeof(char)) - 1>(c)>()

struct nope{
	typedef std::false_type found;
};

std::ostream & operator<<(std::ostream &os, const nope& ){
	return os << "nope!";
}


template<unsigned long long id, typename T>
auto _ref(const T&, const nope&){
	return nope();
}

template<unsigned long long id, Level l, typename T>
auto _ref(const MutableTemporary<id,l,T> &mt, const nope&){
	RefTemporary<l,T> rt(mt);
	return rt;
}

template<unsigned long long id, unsigned long long id2, Level l, typename T>
typename std::enable_if<id != id2,nope>::type
_ref(const MutableTemporary<id2,l,T> &, const nope&){
	return nope();
}

template<unsigned long long, Level l, typename T1, typename T2>
RefTemporary<l,T2> _ref(const T1&, const RefTemporary<l,T2>& r){
	return r;
}

template<typename S, typename W>
auto operator/(const Seq<S,W> &s, const nope&){
	return s;
}

template<typename Seq, unsigned long long id>
auto replace(const Seq &s, const refstr<id> &r){

	auto try2 = s.ifold([](const auto &e, const auto &acc){
			return _ref<id>(e,acc);
		},
		nope());

	ReplaceMe<refstr<id> > rm(r);

	return conditional<std::is_same<decltype(try2),nope>::value>
		(rm, try2);
}

template<typename S, typename W, unsigned long long id>
auto operator/(const Seq<S,W> &s, const refstr<id> &r){
	return s / replace(s,r);
}

template<Level l, typename T>
bool is_reftemp(const RefTemporary<l,T> *){
	return true;
}

template<typename T>
struct is_RefTemporary : std::integral_constant<bool,is_reftemp(mke_p<T>())>::type
{};


REPLACEME_OK(Temporary);
REPLACEME_OK(RefTemporary);
