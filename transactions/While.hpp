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
	static_assert(can_flow(get_level<Cond>::value, max_level<Then>::value),	\
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
			static_assert(min_level<Then>::value != max_level<Then>::value? min_level<Then>::value != Level::undef && max_level<Then>::value != Level::undef : true,"Error: undef shouldn't appear when there are real levels.");
		}
	
	While(const While& w)
		:cond(w.cond),then(w.then){}

	auto handles() const {
		return std::tuple_cat(::handles(cond), stmt_handles(then));
	}

	bool strongCall(StrongCache& c, StrongStore &s) const {
		s.in_loop();
		AtScopeEnd ase{[&](){s.out_of_loop();}};
		discard(ase);
		choose_strong<get_level<Cond>::value> choice1{nullptr};
		choose_strong<min_level<Then>::value> choice2{nullptr};
		bool ret = strongCall(c,s,choice1,choice2);
		return ret;
	}

	bool strongCall(StrongCache& c, StrongStore &s, const std::true_type*,const std::true_type*) const {
		//nothing causal in this while loop. Do it all at once.

		auto new_cache = std::make_unique<StrongCache>();
		while (run_ast_strong(*new_cache,s,cond)) {
			call_all_strong(*new_cache,s,then);
			new_cache = std::make_unique<StrongCache>();
		}
		//TODO: error propogation;
		return true;
	}

	bool strongCall(StrongCache& c_old_mut, StrongStore &s, const std::true_type*, const std::false_type*) const {
		//the "hard" case, if you will. a strong condition, but some causal statements inside.
		c_old_mut.emplace<std::list<std::unique_ptr<StrongCache> > >(id);
		const auto& c_old = c_old_mut;
		
		auto &store_stack = c_old_mut.get<std::list<std::unique_ptr<StrongCache> > >(id);

		store_stack.emplace_back(std::make_unique<StrongCache>());
		assert(store_stack.back().get() != &c_old_mut);
		while(run_ast_strong(*store_stack.back(),s,cond)) {
			call_all_strong(*store_stack.back(),s,then);
			store_stack.emplace_back(std::make_unique<StrongCache>());
			assert(store_stack.front().get() != store_stack.back().get());
		}
		//there's one too many in here.
		store_stack.pop_back();

		assert(c_old.contains(id));

		{
			auto it = store_stack.begin();
			for (auto &c : c_old.get<std::list<std::unique_ptr<CausalCache> > >(id)){
				assert (((void*)c.get()) == ((void*)it->get()));
				assert(c->store_impl == (*it)->store_impl);
				assert(&(c->store_impl) == &((*it)->store_impl));
				++it;
			}
			//Debugging!
		}


		//TODO: error propogation
		return true;
	}

	bool strongCall(StrongCache& c, const StrongStore &s, const std::false_type*, const std::false_type*) const {
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
	
	bool causalCall(CausalCache& c_old, CausalStore &s) const {
		s.in_loop();
		AtScopeEnd ase{[&](){s.out_of_loop();}};
		discard(ase);
		//if there's a cache for this AST node, then
		//that means we've already run the condition.
		//look it up!
		if (c_old.contains(id)){
			//so, hopefully this casting is safe.  If not, use the move constructor.
			for (auto &c : c_old.get<std::list<std::unique_ptr<CausalCache> > >(id)){
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

template<typename Cond, typename Then>
struct chld_min_level<While<Cond,Then> > : level_constant<min_of_levels(get_level<Cond>::value, min_level<Then>::value)> {};

template<typename Cond, typename Then>
struct chld_max_level<While<Cond,Then> > : level_constant<max_of_levels(get_level<Cond>::value, max_level<Then>::value)> {};


template<unsigned long long ID, typename Cond, typename Then>
auto find_usage(const While<Cond,Then>& _while){
	return fold(tuple_cons(_while.cond, _while.then),
				[](const auto &e, const auto &acc){
					return choose_non_np(find_usage<ID>(e),acc);
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


