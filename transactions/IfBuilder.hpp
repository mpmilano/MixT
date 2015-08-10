#pragma once
#include "ConStatement.hpp"
#include "BaseCS.hpp"
#include "ConExpr.hpp"
#include "CommonExprs.hpp"
#include "TransactionBuilder.hpp"
#include <iostream>


#define if_concept(Cond,Then,Els) (										\
																		\
		is_ConStatement<Then>::value									\
		&& is_ConExpr<Cond>::value										\
		&&is_ConExpr<Els>::value										\
																		\
		)

#define if_concept_2(Cond,Then,Els)										\
	((get_level<Cond>::value == Level::causal &&						\
	  get_level<Then>::value == Level::causal &&						\
	  get_level<Els>::value == Level::causal)							\
	 ||																	\
	 (get_level<Cond>::value == Level::strong))


template<typename Cond, typename Then, typename Els>
struct If : public ConStatement<min_level<Then,Els>::value> {

	static constexpr Level level = min_level<Then,Els>::value;
	typedef Cond Cond_t;
	const Cond cond;
	const Then then;
	const Els els;

	If(const Cond& cond, const Then& then, const Els &els):
		cond(cond),then(then),els(els)
		{
			static_assert(if_concept(Cond,Then,Els) && if_concept_2(Cond,Then,Els),
						  "Bad types got to constructor");
		}

	BitSet<HandleAbbrev> getReadSet() const {
		return set_union(get_ReadSet(cond),then.getReadSet(),els.getReadSet());
	}

	bool operator()(Store &s) const {
		static_assert(!is_ConStatement<decltype(cond(s))>::value);
		static_assert(!is_ConStatement<decltype(then(s))>::value);
		return (cond(s) ? then(s) : els(s));
	}
};

template<typename A, typename B, typename C>
constexpr bool is_If_f(const If<A,B,C>*){
	return true;
}

template<typename A>
constexpr bool is_If_f(const A*){
	return false;
}

template<typename T>
struct is_If : std::integral_constant<bool,is_If_f(mke_p<T>())>::type {};


template<typename Cond, typename Then, typename Els>
std::ostream & operator<<(std::ostream &os, const If<Cond,Then,Els>& i){
	return os << "(" << i.cond <<" ? " << i.then << ")";
}

template<typename PrevBuilder, typename Cond, typename Then, typename Els, typename Vars>
struct IfBuilder {
	const PrevBuilder prevBuilder;
	const If<Cond,Then,Els > this_if;
	typedef Cat<prevBuilder::vars,Vars> vars;
	static constexpr Level pc =
		min_level<PrevBuilder::pc,get_level<Cond>::value>::value;
	IfBuilder(const PrevBuilder &pb, const If<Cond,Then,Els> &to)
		:prevBuilder(pb),this_if(to){
		static_assert(is_ConExpr<Cond>::value,
					  "Error: If condition is not an expression.");
		static_assert(can_flow(PrevBuilder::pc,get_level<Cond>::value),
					  "Error: Flow violation in declaration of If."
			);
		//Note to self: the if_concept takes care of flows from
		//the condition to the branches.
		//that *can* be moved here if we want (weak TODO).
	}
};

template<typename PrevBuilder, typename Cond, typename Then, typename Els, typename Vars>
constexpr bool is_IfBuilder_f(const IfBuilder<PrevBuilder, Cond, Then, Els, Vars>*) {
	return true;
}

template<typename T>
constexpr bool is_IfBuilder_f(T*) {
	return false;
}

template<typename T>
struct is_IfBuilder : std::integral_constant<bool, is_ifBuilder_f(mke_p<T>()) >::type {};


template<typename PrevBuilder, typename Cond, typename Then, typename Vars>
struct ThenBuilder : IfBuilder<PrevBuilder,Cond,Then,std::tuple<>, Vars>{
	ThenBuilder(const PrevBuilder &pb, const If<Cond,Then,std::tuple<>> &to)
		:IfBuilder(pb,to) {}

	template<typename T>
	auto operator/(const T &t) const {
		static_assert(is_ConStatement<T>::value, "Error: non-statement in Then clause of If.");
		typedef Cat<Then,std::tuple<T> > newThen;
		If<Cond,newThen,std::tuple<> >
			new_if(this_if.cond,std::tuple_cat(this_if.then, std::make_tuple(t)),std::tuple<>());
		ThenBuilder<PrevBuilder,Cond,newThen,Cat<Vars,all_declarations<T> > > r(this->pb,new_if);
		return r;
	}
};

template<typename PrevBuilder, typename Cond, typename Then, typename Els, typename Vars>
struct ElseBuilder : IfBuilder<PrevBuilder,Cond,Els, Vars>{
	ElseBuilder(const PrevBuilder &pb, const If<Cond,Then,Els> &to)
		:IfBuilder(pb,to) {}

	template<typename T>
	auto operator/(const T &t) const {
		static_assert(is_ConStatement<T>::value, "Error: non-statement in Else clause of If.");
		typedef Cat<Els,std::tuple<T> > newEls;
		If<Cond,Then,newEls >
			new_if(this_if.cond,this_if.then, std::tuple_cat(this_if.els, std::make_tuple(t)));
		ElseBuilder<PrevBuilder,Cond,Then,newEls,Cat<Vars,all_declarations<T> > > r(this->pb,new_if);
		return r;
	}
}

template<typename Cond>
struct IfBegin {
	const Cond c;
};

struct IfEnd {};
	
template<typename PrevBuilder, typename Cond>
auto append(const PrevBuilder &pb, const IfBegin<Cond> &ib){
	NAME_CHECK(PrevBuilder::vars, Cond);
	If<Cond,std::tuple<>,std::tuple<> > if_elem(ib.c,std::tuple<>(),std::tuple<>());
	ThenBuilder<PrevBuilder, Cond, std::tuple<>, std::tuple<> > r(pb,if_elem);
	return r;
}

template<typename CurrBuilder>
auto append(const CurrBuilder &pb, const IfEnd<Cond> &){
	static_assert(is_IfBuilder<CurrBuilder>::value,
				  "Error: attempt to end If when not in if context! This is a framework bug, please file.");
	return append(pb.prevBuilder, pb.this_if);
}

template<typename Cons>
auto make_if_begin(const Cons &s){
	IfBegin<Cons> r{s};
	return r;
}

template<typename Cons>
const IfEnd& make_if_end(const Cons &s){
	static constexpr IfEnd e;
	return e;
}
