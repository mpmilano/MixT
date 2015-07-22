#pragma once
#include "ConStatement.hpp"
#include "BaseCS.hpp"
#include "ConExpr.hpp"
#include "CommonExprs.hpp"
#include "Seq.hpp"
#include <iostream>


template<typename Cond, typename Then>
struct If;

#define handle_level backend::handle_level

#define if_concept(Cond,Then) ( \
																		\
		is_ConStatement<Then>::value									\
		&& is_ConExpr<Cond>::value										\
																		\
		)

#define if_concept_2(Cond,Then) \
	((get_level<Cond>::value == Level::causal &&						\
	  get_level<Then>::value == Level::causal)							\
	 ||																	\
	 (get_level<Cond>::value == Level::strong))

template<typename A, typename B, typename C, typename D>
auto operator/(const Seq<A,B>& a, const If<C,D> &i){
	return seq_cat(a,make_seq(i));
}

template<backend::Level l, typename T>
auto level_change(const RefTemporary<l,T> &t){
	return std::make_pair(Noop<l>(),t);
}

template<backend::Level l, backend::Level lold, typename T, restrict(get_level<T>::value != l)>
auto level_change(const RefTemporary<lold,RefTemporary<l,T> > &t){
	return std::make_pair(Noop<l>(),t.t.t);
}

template<backend::Level l, backend::Level lold, typename T, restrict(get_level<T>::value != l)>
auto level_change(const RefTemporary<lold,T> &t){
	auto tmp = make_temp<l>(t);
	auto rt = ref_temp(tmp);
	return std::make_pair(tmp,rt);
}

template<typename A, typename B, typename C, typename D>
auto operator/(const Seq<A,B>& a, const If<C,ReplaceMe<D> > &i){
	auto repl = replace(a,i.then.t);
	auto nl = level_change<get_level<decltype(repl)>::value>(i.cond);
	return seq_cat (a / nl.first, make_seq(If<decltype(nl.second),decltype(repl)>(nl.second,repl)));
}

template<backend::Level l, typename T, typename Then>
auto make_if(const RefTemporary<l,T>& c, const Then &t){
	return If<RefTemporary<l,T>,Then> (c,t);
}

template<backend::Level l, typename Then>
Then make_if(const DummyConExpr<l>&, const Then &t){
	return t;
}

template<typename Cond, typename Then,
		 restrict(if_concept(Cond,Then))>
auto make_if(const Cond& c, const Then &t){
	static_assert(if_concept_2(Cond,Then), "Failure: consistency violation.");
	static_assert(!is_RefTemporary<Cond>::value,"Failure: I got overloading wrong");
	
	auto temp = make_temp<get_level<Then>::value>(c);
	return make_seq(temp) / make_if(temp,t);
}

template<backend::Level l, typename Then1, typename Then2>
Seq<Then1,Then2> make_if(const DummyConExpr<l>& , const Seq<Then1, Then2> &t){
	return t;
}

template<typename Cond, typename Then1, typename Then2>
auto make_if(const Cond& c, const Seq<Then1, Then2> &t){
	auto temp_strong = make_temp<backend::Level::strong>(c);
	auto temp_weak = make_temp<backend::Level::causal>(ref_temp(temp_strong));
	return make_seq(temp_strong) /
		temp_weak /
		fold(t.strong,
			 [&](const auto &a, const auto &acc){
				 auto acc2 = acc /(make_if(ref_temp(temp_strong),a));
				 return acc2;
			 },empty_seq()) /
		fold(t.weak,
			 [&](const auto &a, const auto &acc){
				 auto acc2 = acc /(make_if(ref_temp(temp_weak),a));
				 return acc2;
			 },empty_seq());
}


template<typename Cond2, typename Then1, typename Then2>
auto make_if(const Cond2& c,
			 const std::initializer_list<Seq<Then1, Then2> > &t){
	return make_if(c,*(t.begin()));
}

template<backend::Level, typename Els>
struct Else {
	const Els &e;
};

template<backend::Level l,typename Els>
auto make_else(const Els &e){
	Else<l,Els> ret{e};
	return ret;
}

template<backend::Level l>
DummyConExpr<l> make_else(const DummyConExpr<l> &e){
	return e;
}

template<typename T1, typename T2, typename T3>
auto make_if(const T1& t1, const T2 &t2, const T3 &t3){
	auto temp = make_temp<max_level<T3>::value>(t1);
	return make_if(ref_temp(t1),t2) / make_if(make_not(ref_temp(temp)),t3);
}

template<typename Els, typename Str, typename... Stuff>
auto operator_append_impl(const Seq<Str, std::tuple<Stuff...> > &s,
						  const Else<backend::Level::causal,Els> & e){
	static_assert(sizeof...(Stuff) > 0);
	typedef typename last_of<Stuff...>::type LastIf;
	static constexpr int last_index = sizeof...(Stuff) - 1;
	auto not_temp = make_temp<backend::Level::causal>(
		make_not(
			std::get<last_index>(s.weak).cond));
	return s / not_temp / make_if(ref_temp(not_temp),e.e);
}


template<typename Els, typename Wk, typename... Stuff>
auto operator_append_impl(const Seq<std::tuple<Stuff...>, Wk> &s,
						  const Else<backend::Level::strong,Els> & e){
	static_assert(sizeof...(Stuff) > 0);
	typedef typename last_of<Stuff...>::type LastIf;
	static constexpr int last_index = sizeof...(Stuff) - 1;
	auto not_temp = make_temp<backend::Level::strong>(
		make_not(
			std::get<last_index>(s.strong).cond));
	return s / not_temp / make_if(ref_temp(not_temp),e.e);
}

template<typename Els, typename Str, typename... Stuff> /*
typename std::enable_if<sizeof...(Stuff) != 0,
	decltype(operator_append_impl(mke<Seq<Str,std::tuple<Stuff...> > >(),
								  mke<Else<backend::Level::causal, Els> > ))>::type
//*/auto
operator/(const Seq<Str, std::tuple<Stuff...> > &s,
		  const Else<backend::Level::causal,Els> & e){
	return operator_append_impl(s,e);
}

template<typename Els, typename Wk, typename... Stuff> /*
typename std::enable_if<sizeof...(Stuff) != 0,
						decltype(operator_append_impl
								 (mke<Seq<std::tuple<Stuff...>, Wk > >(),
								  mke<Else<backend::Level::strong, Els> > ))>::type
//*/auto
operator/(const Seq<std::tuple<Stuff...>, Wk> &s,
		  const Else<backend::Level::strong,Els> & e){
	return operator_append_impl(s,e);
}


template<typename Cond, typename Then>
struct If : public ConStatement<get_level<Then>::value> {

	static constexpr Level level = get_level<Then>::value;
	typedef Cond Cond_t;
	const Cond cond;
	const Then then;

	If(const Cond& cond, const Then& then):
		cond(cond),then(then)
		{
			static_assert(if_concept(Cond,Then) && if_concept_2(Cond,Then),
						  "Bad types got to constructor");
		}

	CONNECTOR_OP

	template<typename Els>
	auto operator/(const Else<level,Els> &e) const {
		return make_seq(*this) / make_if(make_not(cond),e.e);
	}

	BitSet<backend::HandleAbbrev> getReadSet() const {
		return set_union(get_ReadSet(cond),then.getReadSet());
	}

	auto operator()(Store &s) const {
		static_assert(!is_ConStatement<decltype(cond(s))>::value);
		static_assert(!is_ConStatement<decltype(then(s))>::value);
		return (cond(s) ? then(s) : Noop<level>().operator()(s));
	}

	
	template<typename Cond2, typename Then2, typename ignore>
	friend If<Cond2,Then2> make_if(const Cond2& , const Then2 &);

	template<typename Cond2, typename Then2>
	friend std::ostream & operator<<(std::ostream &os, const If<Cond2,Then2>& i);
};

template<typename A, typename B>
constexpr bool is_If_f(const If<A,B>*){
	return true;
}

template<typename A>
constexpr bool is_If_f(const A*){
	return false;
}

template<typename T>
struct is_If : std::integral_constant<bool,is_If_f(mke_p<T>())>::type {};


template<typename Cond, typename Then>
std::ostream & operator<<(std::ostream &os, const If<Cond,Then>& i){
	return os << "(" << i.cond <<" ? " << i.then << ")";
}

#define IF make_if(
#define THEN ,
#define ELSE(x) ) / make_else<backend::Level::x>(
#define FI ) /


template<typename C, typename T>
constexpr bool verify_ReplaceMe(const If<C,ReplaceMe<T> >*){
	constexpr bool dummy = get_level<ReplaceMe<T> >::value == backend::Level::causal &&
		get_level<ReplaceMe<T> >::value == backend::Level::strong;
	static_assert(dummy || !dummy, "NameError: Failed to replace for reference");
	return false;
}

