#pragma once
#include "ConStatement.hpp"
#include "BaseCS.hpp"
#include "ConExpr.hpp"
#include "CommonExprs.hpp"
#include "Operate.hpp"
#include <iostream>
#include <list>

namespace myria { namespace mtl {

#define while_concept(Cond,Then)										\
		{static_assert(is_ConExpr<Cond>::value, "Error: while-condition not a condition"); \
			static_assert(is_cs_tuple<Then>::value, "Error: while body not a tuple of statements"); }

#define while_concept_2(Cond,Then)										\
		static_assert(can_flow(get_level<Cond>::value, max_level<Then>::value), \
					  "Error: implicit flow found in While.")


		template<typename Cond, typename Then>
		struct While : public ConStatement<min_level<Then>::value> {


			typedef Cond Cond_t;
			const Cond cond;
			const Then then;
			const int id = mutils::gensym();

			While(const Cond& cond, const Then& then):
				cond(cond),then(then)
				{
					while_concept(Cond,Then);
					while_concept_2(Cond,Then);
					static_assert(min_level<Then>::value != max_level<Then>::value? min_level<Then>::value != Level::undef && max_level<Then>::value != Level::undef : true,"Error: undef shouldn't appear when there are real levels.");
				}
	
			While(const While& w)
				:cond(w.cond),then(w.then){}

			auto environment_expressions() const {
				return std::tuple_cat(mtl::environment_expressions(cond), stmt_environment_expressions(then));
			}

			bool strongCall(TransactionContext* ctx, StrongCache& c, StrongStore &s) const {
				choose_strong<get_level<Cond>::value> choice1{nullptr};
				choose_strong<min_level<Then>::value> choice2{nullptr};
				bool ret = strongCall(ctx,c,s,choice1,choice2);
				return ret;
			}

			bool strongCall(TransactionContext* ctx, StrongCache&, StrongStore &s, const std::true_type*,const std::true_type*) const {
				//nothing causal in this while loop. Do it all at once.

				auto new_cache = std::make_unique<StrongCache>();
				while (run_ast_strong(ctx,*new_cache,s,cond)) {
					call_all_strong(ctx,*new_cache,s,then);
					new_cache = std::make_unique<StrongCache>();
				}
				//TODO: error propogation;
				return true;
			}

			bool strongCall(TransactionContext* ctx, StrongCache& c_old_mut, StrongStore &s, const std::true_type*, const std::false_type*) const {
				//the "hard" case, if you will. a strong condition, but some causal statements inside.
				auto cache_stack_p = std::make_unique<std::list<std::unique_ptr<StrongCache> > >();
				auto& cache_stack = *cache_stack_p;
				c_old_mut.emplace<decltype(cache_stack_p)>(id,cache_stack_p.release());
				const auto& c_old = c_old_mut;

				cache_stack.emplace_back(std::make_unique<StrongCache>());

				assert(cache_stack.back().get() != &c_old_mut);
				while(run_ast_strong(ctx,*cache_stack.back(),s,cond)) {
					call_all_strong(ctx,*cache_stack.back(),s,then);
					cache_stack.emplace_back(std::make_unique<StrongCache>());

					assert(cache_stack.front().get() != cache_stack.back().get());
				}
				//there's one too many in here.
				cache_stack.pop_back();

				assert(c_old.contains(id));

				{
					auto it = cache_stack.begin();
					for (auto &c : *c_old.get<std::unique_ptr<std::list<std::unique_ptr<StrongCache> > > >(id)){
						assert (((void*)c.get()) == ((void*)it->get()));
						++it;
					}
					//Debugging!
				}
				//TODO: error propogation
				return true;
			}

			bool strongCall(TransactionContext* ctx, StrongCache& c, const StrongStore &s, const std::false_type*, const std::false_type*) const {
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
				run_ast_strong(ctx,s,cond);
				return call_all_strong(ctx,c,s,then);
			}
	
			bool causalCall(TransactionContext* ctx, CausalCache& c_old, CausalStore &s) const {
				//if there's a cache for this AST node, then
				//that means we've already run the condition.
				//look it up!
				if (c_old.contains(id)){
					const auto& force_cache_const = c_old;
					for (const auto &cs : *force_cache_const.get<std::unique_ptr<std::list<std::unique_ptr<StrongCache> > > >(id)){
						CausalCache cc{*cs};
						call_all_causal(ctx,cc,s,then);
					}
				}
				else {
					//causal condition, so nothing interesting here.
					auto new_cache = std::make_unique<CausalCache>();
					while (run_ast_causal(ctx,*new_cache,s,cond)) {
						call_all_causal(ctx,*new_cache,s,then);
						new_cache = std::make_unique<CausalCache>();
					}
					//TODO: error propogation;
					return true;
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
		struct contains_temporary<ID, While<Cond,Then> > : contains_temp_fold<ID,mutils::Cat<std::tuple<Cond>,Then> > {};

		template<typename A, typename B>
		constexpr bool is_While_f(const While<A,B>*){
			return true;
		}

		template<typename A>
		constexpr bool is_While_f(const A*){
			return false;
		}

		template<typename T>
		struct is_while : std::integral_constant<bool,is_While_f(mutils::mke_p<T>())>::type {};



	} }
