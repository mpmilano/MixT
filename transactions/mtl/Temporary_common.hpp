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
		
		template<unsigned long long ID, Level l, typename T>
		struct TemporaryCommon : public GeneralTemp, public ConStatement<get_level<T>::value> {
			static_assert(is_ConExpr<T>::value,
						  "Error: can only assign temporary the result of expressions");
	
			static_assert(can_flow(get_level<T>::value,l),"Error: flow violation");

			const T t;
			const int store_id;
			TemporaryCommon(const std::string name, const T& t):GeneralTemp(name,mutils::to_string(t)),t(t),store_id(std::hash<std::string>()(name)){
				static_assert(get_level<TemporaryCommon>::value == get_level<T>::value,"error: you overrode get_level wrong for Temporaries");
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
		struct chld_min_level<TemporaryCommon<ID,l,Temp> > : level_constant<l> {};

		template<unsigned long long ID, Level l, typename Temp>
		struct chld_max_level<TemporaryCommon<ID,l,Temp> > : level_constant<l> {};

		

	}}
