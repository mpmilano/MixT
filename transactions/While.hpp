#pragma once
#include "ConStatement.hpp"
#include "BaseCS.hpp"
#include "ConExpr.hpp"
#include "CommonExprs.hpp"
#include "Operate.hpp"
#include <iostream>

#define while_concept(Cond,Then)										\
	{static_assert(is_ConExpr<Cond>::value, "Error: while-condition not a condition"); \
		static_assert(is_cs_tuple<Then>::value, "Error: while body not a tuple of statements"); }

#define while_concept_2(Cond,Then)										\
	static_assert((get_level<Cond>::value == Level::causal &&			\
				   max_level<Then>::value == Level::causal)				\
				  ||													\
				  (get_level<Cond>::value == Level::strong),			\
				  "Error: implicit flow found in While.")


template<typename Cond, typename Then>
struct While : public ConStatement<min_level<Then>::value> {


	typedef Cond Cond_t;
	const Cond cond;
	const Then then;
	const int id = gensym();

	While(const Cond& cond, const Then& then):
		cond(cond),then(then)
		{
			while_concept(Cond,Then);
			while_concept_2(Cond,Then);
		}

	BitSet<HandleAbbrev> getReadSet() const {
		std::cerr << "TODO: split into strong + weak? " << std::endl;
		//return set_union(get_ReadSet(cond),then.getReadSet(),els.getReadSet());
		return 0;
	}
	
	bool strongCall(Store &c, Store &s) const {
		static_assert(!is_ConStatement<decltype(run_ast(s,cond))>::value);
		static_assert(!is_ConStatement<decltype(call_all(s,then))>::value);
		std::integral_constant<bool,get_level<Cond>::value == Level::strong>*
			choice1{nullptr};
		std::integral_constant<bool,min_level<Then>::value == Level::strong>*
			choice2{nullptr};
		return strongCall(c,s,choice);
	}

	bool strongCall(Store &c_old, Store &s, const std::true_type*,const std::true_type*) const {
		//nothing causal in this while loop. Do it all at once.
		while (run_ast_strong(s,cond)) call_all_strong(s,then);
		
		return (run_ast_strong(s,cond) ? call_all_strong(s,then) : call_all_strong(s,els));
	}

	bool strongCall(Store &c, const Store &s, const std::true_type*, const std::false_type*) const {
		//the "hard" case, if you will. a strong condition, but some causal statements inside.
		Store *c = &c_old;
		while(run_ast_strong(s,cond)){
		}		
	}

	bool strongCall(Store &c, const Store &s, const std::false_type*, const std::false_type*) const {
		//there aren't any strong mutative things in here.
		//TODO: if I ban anti-dependencies? 
	}
	
	bool causalCall(Store &c, Store &s) const {
		//if there's a cache for this AST node, then
		//that means we've already run the condition.
		//look it up!
	}

	
};

template<unsigned long long ID, typename Cond, typename Then>
auto find_usage(const While<Cond,Then>& _while){
	return fold(tuple_cons(_while.cond, _while.then),
				[](const auto &e, const auto &acc){
					return pick_useful(find_usage<ID>(e),acc);
				}
				, nullptr);
}

template<unsigned long long ID, typename Cond, typename Then>
struct contains_temporary<ID, While<Cond,Then> > : contains_temp_fold<ID,Cat<std::tuple<Cond>,Then> > {};

template<typename A, typename B>
constexpr bool is_While_f(const While<A,B>*){
	return true;
}

template<typename A>
constexpr bool is_While_f(const A*){
	return false;
}

template<typename T>
struct is_while : std::integral_constant<bool,is_While_f(mke_p<T>())>::type {};


template<typename Cond, typename Then>
std::ostream & operator<<(std::ostream &os, const While<Cond,Then>& i){
	return os << "while (" << i.cond <<") do {" << i.then << "}";
}
