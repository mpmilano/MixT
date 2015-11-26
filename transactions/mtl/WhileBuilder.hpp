#pragma once
#include "ConStatement.hpp"
#include "BaseCS.hpp"
#include "ConExpr.hpp"
#include "CommonExprs.hpp"
#include "TransactionBuilder.hpp"
#include "While.hpp"

namespace myria { namespace mtl {


template<typename PrevBuilder, typename Cond, typename Then>
struct WhileBuilder : public Base_Builder{
	const PrevBuilder prevBuilder;
	const While<Cond,Then > this_while;
	typedef typename PrevBuilder::pc old_pc;
	typedef std::integral_constant<
		Level, min_level<old_pc,Cond>::value> pc;
	WhileBuilder(const PrevBuilder &pb, const While<Cond,Then> &to)
		:prevBuilder(pb),this_while(to){
		static_assert(is_ConExpr<Cond>::value,
					  "Error: While condition is not an expression.");
		static_assert(can_flow(PrevBuilder::pc::value,get_level<Cond>::value),
					  "Error: Flow violation in declaration of While."
			);
		static_assert(is_builder<PrevBuilder>(), "Error: PrevBuilder is not a builder!");
		//Note to self: the while_concept takes care of flows from
		//the condition to the branches.
		//that *can* be moved here if we want (weak TODO).
	}
};

template<typename PrevBuilder, typename Cond, typename Then>
constexpr bool is_WhileBuilder_f(const WhileBuilder<PrevBuilder, Cond, Then>*) {
	return true;
}


template<typename T>
struct is_WhileBuilder : std::integral_constant<bool, is_WhileBuilder_f(mutils::mke_p<T>()) >::type {};


template<typename PrevBuilder, typename Cond, typename Then>
struct WhileBodyBuilder : WhileBuilder<PrevBuilder,Cond,Then>{
	WhileBodyBuilder(const PrevBuilder &pb, const While<Cond,Then> &to)
		:WhileBuilder<PrevBuilder,Cond,Then >(pb,to) {
		static_assert(is_builder<PrevBuilder>(), "Error: PrevBuilder is not a builder!");
	}

	template<typename T>
	auto operator/(const T &t) const {
		static_assert(is_ConStatement<T>::value,
					  "Error: non-statement in Then clause of While.");
		typedef mutils::Cat<Then,std::tuple<T> > newThen;
		While<Cond,newThen >
			new_while(this->this_while.cond,
				   std::tuple_cat(this->this_while.then,
								  std::make_tuple(t)));
		WhileBodyBuilder<PrevBuilder,Cond,newThen >
			r(this->prevBuilder,new_while);
		return r;
	}
};

template<typename Cond>
struct WhileBegin {
	const Cond c;
};

struct WhileEnd { constexpr WhileEnd(){} };
	
template<typename PrevBuilder, typename Cond>
auto append(const PrevBuilder &pb, const WhileBegin<Cond> &ib){
	While<Cond,std::tuple<> > while_elem(ib.c,std::tuple<>());
	WhileBodyBuilder<PrevBuilder, Cond, std::tuple<> > r(pb,while_elem);
	return r;
}

//TODO: else. Support doesn't even exist in the macros right now.
//TODO: else "watcher:" if the next thing in sequence is an else_begin,
//continue the if.  Else, behave exactly as while_end.

template<typename CurrBuilder>
auto append(const CurrBuilder &pb, const WhileEnd&){
	static_assert(is_WhileBuilder<CurrBuilder>::value,
				  "Error: attempt to end While when not in if context! This is a framework bug, please file.");
	return append(pb.prevBuilder, pb.this_while);
}

template<typename Cons>
auto make_while_begin(const Cons &s){
	WhileBegin<Cons> r{s};
	return r;
}

const WhileEnd& make_while_end();

	} }
