#pragma once
#include "ConExpr.hpp"
#include "CommonExprs.hpp"
#include <string>

namespace myria { namespace mtl {

		struct GeneralTemp {
			const std::string name;
			const std::string gets;
			GeneralTemp(const std::string &n, const std::string &g):name(n),gets(g) {}
		};

		/*  TODO: redesign.
			Pursuant to a conversation with ACM, i should replace the let_ifValid and let_Mutable constructs with new constructs more focused on referencing/dereferencing.
			let_ifValid() becomes a dereferencing bind; the bound variable mutates the remote object when assigned to, and is drefd in free_exprs and operations
			let_mutable() becomes a box-bind; the bound variable mutates the pointer itself, and is preserved as a raw handle in free_exprs and operations
			Note: we should likely change the syntax of free_expr; no need to "dref" a handle syntactically, as that behavior is now dependant on how it was bound.
			Note: this should be a large re-write (if done correctly).  Will defer until after current context work and store implementations.
		*/


		//the level here is for referencing the temporary later.
		//it's the endorsement check!
		template<unsigned long long ID, Level l, typename T>
		struct Temporary : public GeneralTemp, public ConStatement<get_level<T>::value> {
			static_assert(is_ConExpr<T>::value,
						  "Error: can only assign temporary the result of expressions");
	
			static_assert(can_flow(get_level<T>::value,l),"Error: flow violation");

			const T t;
			const int store_id;
			Temporary(const std::string name, const T& t):GeneralTemp(name,mutils::to_string(t)),t(t),store_id(std::hash<std::string>()(name)){
				static_assert(get_level<Temporary>::value == get_level<T>::value,"error: you overrode get_level wrong for Temporaries");
			}

			auto environment_expressions() const {
				return mtl::environment_expressions(t);
			}

			auto strongCall(TransactionContext* ctx, StrongCache& c, StrongStore &s) const {

				choose_strong<get_level<T>::value > choice{nullptr};
				return strongCall(ctx,c,s,choice);
			}

			auto strongCall(TransactionContext* ctx, StrongCache& c, StrongStore &s, std::true_type*) const {
				typedef typename std::decay<decltype(run_ast_strong(ctx,c,s,t))>::type R;
				//TODO: when we do the great preserve replacements, this (and many other sites)
				//should change to acknowledge them. Note: we're hoping here that if the
				//expression to which this temporary is being assigned drefs a handle,
				//that this target expression will actually change the context back to
				//something more appropriate.
				assert(ctx);
				auto prev = ctx->execution_context;
				if (is_handle<R>::value) ctx->execution_context = context::t::data;
				s.emplace<R>(store_id, run_ast_strong(ctx,c,s,t));
				if (is_handle<R>::value) ctx->execution_context = prev;
				return true;
			}

			void strongCall(TransactionContext* ctx, StrongCache& c, const StrongStore &s, std::false_type*) const {
				typedef typename std::decay<decltype(run_ast_strong(ctx,c,s,t))>::type R;
				auto prev = ctx->execution_context;
				assert(ctx);
				if (is_handle<R>::value) ctx->execution_context = context::t::data;
				run_ast_strong(ctx,c,s,t);
				if (is_handle<R>::value) ctx->execution_context = prev;
			}

			auto causalCall(TransactionContext* ctx, CausalCache& c, CausalStore &s) const {
				choose_causal<get_level<T>::value > choice{nullptr};
				return causalCall(ctx,c,s,choice);
			}

			auto causalCall(TransactionContext* ctx, CausalCache& c, CausalStore &s,std::true_type*) const {
				typedef typename std::decay<decltype(run_ast_causal(ctx,c,s,t))>::type R;
				s.emplace<R>(store_id,run_ast_causal(ctx,c,s,t));
				return true;
			}

			auto causalCall(TransactionContext* ctx, CausalCache& c, CausalStore &s,std::false_type*) const {
				//noop.  We've already executed this instruction.
				return true;
			}

		};

		template<unsigned long long ID, Level l, typename Temp>
		struct chld_min_level<Temporary<ID,l,Temp> > : level_constant<l> {};

		template<unsigned long long ID, Level l, typename Temp>
		struct chld_max_level<Temporary<ID,l,Temp> > : level_constant<l> {};

		template<unsigned long long ID, Level l, typename T>
		auto find_usage(const Temporary<ID,l,T> &rt){
			return mutils::shared_copy(rt);
		}

		template<unsigned long long ID, unsigned long long ID2, Level l, typename T>
		struct contains_temporary<ID, Temporary<ID2,l,T> > : std::integral_constant<bool, ID == ID2> {};

	} }
