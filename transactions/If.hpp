#pragma once
#include "ConStatement.hpp"
#include "BaseCS.hpp"
#include "ConExpr.hpp"
#include "CommonExprs.hpp"
#include "Operate.hpp"
#include <iostream>

#define if_concept(Cond,Then,Els)										\
	{static_assert(is_ConExpr<Cond>::value, "Error: if-condition not a condition"); \
	static_assert(is_cs_tuple<Then>::value, "Error: then-branch not a tuple of statements"); \
	static_assert(is_cs_tuple<Els>::value, "Error: else-branch not a tuple of statements");} \

#define if_concept_2(Cond,Then,Els)										\
	static_assert(can_flow(get_level<Cond>::value,max_level<Then>::value) \
				  && can_flow(get_level<Cond>::value,max_level<Els>::value), \
				  "Error: implicit flow found in IF.")


template<typename Cond, typename Then, typename Els>
struct If : public ConStatement<min_level<typename min_level<Then>::type,
										  typename min_level<Els>::type >::value> {

	typedef Cond Cond_t;
	const Cond cond;
	const Then then;
	const Els els;
	const int id = gensym();

	If(const Cond& cond, const Then& then, const Els &els):
		cond(cond),then(then),els(els)
		{
			if_concept(Cond,Then,Els);
			if_concept_2(Cond,Then,Els);
		}

	If(const If& i):cond(cond),then(then),els(els){}

	auto handles() const {
		return std::tuple_cat(
			cond.handles(),
			stmt_handles(then),
			stmt_handles(els));
	}

	bool strongCall(Cache& c, Store &s) const {
		choose_strong<get_level<Cond>::value> choice{nullptr};
		return strongCall(c,s,choice);
	}

	bool strongCall(Cache& c, Store &s, const std::true_type*) const {
		return (run_ast_strong(c,s,cond) ? call_all_strong(c,s,then) : call_all_strong(c,s,els));
	}

	//just caching can happen here;
	//any mutative action would violate information flow.
	bool strongCall(Cache& c, Store &s, const std::false_type*) const {
		run_ast_strong(c,s,cond);
		return (call_all_strong(c,s,then) &&
				call_all_strong(c,s,els));
	}
	
	bool causalCall(Cache& c, Store &s) const {
		return (run_ast_causal(c,s,cond) ? call_all_causal(c,s,then) : call_all_causal(c,s,els));
	}
	
};

template<typename Cond, typename Then, typename Els>
struct chld_min_level<If<Cond,Then,Els> > : level_constant<min_of_levels(get_level<Cond>::value, min_level<Then>::value, min_level<Els>::value )> {};

template<typename Cond, typename Then, typename Els>
struct chld_max_level<If<Cond,Then,Els> > : level_constant<max_of_levels(get_level<Cond>::value, max_level<Then>::value, max_level<Els>::value )> {};

template<typename A, typename B, typename C>
auto make_if(const A &a, const B &b, const C &c){
	return If<A,B,C>(a,b,c);
}

template<unsigned long long ID, typename Cond, typename Then, typename Els>
auto find_usage(const If<Cond,Then,Els>& _if){
	return fold(std::tuple_cat(std::make_tuple(_if.cond), _if.then, _if.els),
				[](const auto &e, const auto &acc){
					return choose_non_np(find_usage<ID>(e),acc);
				}
				, nullptr);
}

template<unsigned long long ID, typename Cond, typename Then, typename Els>
struct contains_temporary<ID, If<Cond,Then,Els> > : contains_temp_fold<ID,Cat<std::tuple<Cond>,Then,Els> > {};

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

