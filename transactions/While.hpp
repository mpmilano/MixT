#pragma once
#include "ConStatement.hpp"
#include "BaseCS.hpp"
#include "ConExpr.hpp"
#include "CommonExprs.hpp"
#include "Operate.hpp"
#include <iostream>
#include <list>

#define while_concept(Cond,Then)										\
	{static_assert(is_ConExpr<Cond>::value, "Error: while-condition not a condition"); \
		static_assert(is_cs_tuple<Then>::value, "Error: while body not a tuple of statements"); }

#define while_concept_2(Cond,Then)										\
	static_assert((get_level<Cond>::value == Level::causal &&			\
				   max_level<Then>::value == Level::causal && false)	\
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
		std::integral_constant<bool,get_level<Cond>::value == Level::strong>*
			choice1{nullptr};
		std::integral_constant<bool,min_level<Then>::value == Level::strong>*
			choice2{nullptr};
		return strongCall(c,s,choice1,choice2);
	}

	bool strongCall(Store &c, Store &s, const std::true_type*,const std::true_type*) const {
		//nothing causal in this while loop. Do it all at once.
		while (run_ast_strong(c,s,cond)) call_all_strong(c,s,then);

		//TODO: error propogation;
		return true;
	}

	bool strongCall(Store &c_old, Store &s, const std::true_type*, const std::false_type*) const {
		//the "hard" case, if you will. a strong condition, but some causal statements inside.
		s.emplace<std::list<std::unique_ptr<Store> > >(id);
		
		auto &store_stack = s.get<std::list<std::unique_ptr<Store> > >(id);
		
		while(run_ast_strong(*store_stack.back(),s,cond)){
			store_stack.emplace_back(new Store(&c_old));
			call_all_strong(*store_stack.back(),s,then);
		}

		//TODO: error propogation
		return true;
	}

	bool strongCall(Store &c, const Store &s, const std::false_type*, const std::false_type*) const {
		//there aren't any strong mutative things in here.
		//what if there are strong references?
		//they can't be mutative,
		//but FreeExprs could be written such that they depend on the iteration of the loops.
		//example:
		/*
		  causal<int> guard = 0;
		  while(guard < 20){
		    if (free_expr(strong_handle,guard,strong_handle.get(guard))){
			  do_op(Log_action,causal_handle);
			}
		  }
		 */
		//this is fine, however; the FreeExpr itself would be causal, which means
		//that its strong-execution behaviour would just cache its strong arguments,
		//which in this case is exactly what we want.
		run_ast_strong(c,s,cond);
		return call_all_strong(c,s,then);
	}
	
	bool causalCall(Store &c_old, Store &s) const {
		//if there's a cache for this AST node, then
		//that means we've already run the condition.
		//look it up!
		if (c_old.contains(id)){
			for (auto &c : c_old.get<std::list<std::unique_ptr<Store> > >(id)){
				call_all_causal(*c,s,then);
			}
		}
		else {
			//causal condition, so nothing interesting here.
			while(run_ast_causal(c_old,s,cond)) call_all_causal(c_old,s,then);
		}

		return true;
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
