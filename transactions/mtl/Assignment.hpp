#pragma once
#include "Handle.hpp"
#include "Temporary.hpp"
#include "RefTemporary.hpp"
#include "HandleCaching.hpp"
#include "EnvironmentExpression.hpp"

namespace myria { namespace mtl {

		template<typename T, typename Expr>
		struct Assignment : public ConStatement<get_level<Expr>::value>{
		private:

			template<Level l2, HandleAccess ha2>
			static auto hndle_get(TransactionContext* ctx, Handle<l2,ha2,T> h){
				return h.get(ctx->trackingContext->trk,ctx);
			}

			template<typename T2, restrict(!is_handle<T2>::value)>
			static auto hndle_get(TransactionContext*, const T2 &t2){
				return t2;
			}

			template<typename T2>
			struct HDref_i {using t = T2;};
			template<typename T2, Level l2, HandleAccess ha2>
			struct HDref_i<Handle<l2,ha2,T2> > {using t = T2;};
			template<typename T2>
			using HDref = typename HDref_i<run_result<T2> >::t;

			template<typename T2>
			struct is_string : public std::is_same<T2, std::string> {};
	
		public:
			const T t;
			const Expr e;
			const int id = mutils::gensym();
			Assignment(const T &t, const Expr & e)
				:t(t),e(e)
				{
					static_assert(is_handle<run_result<T> >::value,"Error: Assignment construct only for modifying handles!");
					static_assert(
						can_flow(get_level<Expr>::value, get_level<run_result<T> >::value),
						"Error: assignment to strong member from causal expression!"
						);
					static_assert(
						can_flow(get_level<Expr>::value, get_level<T>::value),
						"Error: deduction of assignment LHS requires strong context; causal expression forces causal context!"
						);
					static_assert(mutils::error_helper<is_string, HDref<Expr> >::value,"");
					static_assert(mutils::error_helper<is_string, HDref<T> >::value,"");
					static_assert(std::is_same<HDref<Expr>, HDref<T> >::value, "Error: Assignment of incompatible types (no subtyping applies here)" );
				}

			auto environment_expressions() const{
				return std::tuple_cat(mtl::environment_expressions(e),mtl::environment_expressions(t));
			}

			bool strongCall(TransactionContext* ctx, StrongCache& c, const StrongStore &s) const {
				choose_strong<get_level<T>::value > choice1{nullptr};
				choose_strong<get_level<Expr>::value> choice2{nullptr};
				choose_strong<get_level<run_result<T> >::value> choice3{nullptr};
				strongCall(ctx, c,s,choice1,choice2,choice3);
				return true;
			}

			void strongCall(TransactionContext* ctx, StrongCache &c, const StrongStore &s, std::false_type*, std::false_type*, std::false_type*) const {
				run_ast_strong(ctx,c,s,t);
				run_ast_strong(ctx, c,s,e);
			}
			void strongCall(TransactionContext* ctx, StrongCache &c, const StrongStore &s, std::false_type*, std::true_type*, std::false_type*) const {
				run_ast_strong(ctx,c,s,t);
				c.insert(id,Assignment::hndle_get(run_ast_strong(ctx, c,s,e)));
			}

			template<typename T2>
			auto strongCall(TransactionContext* ctx, StrongCache &c, const StrongStore &s, std::true_type*, T2*, std::true_type*) const {
				static_assert(runs_with_strong(get_level<Expr>::value),"error: flow violation in assignment");
				auto hndl = run_ast_strong(ctx,c,s,t);
				hndl.put(ctx->trackingContext->trk,ctx,Assignment::hndle_get(ctx,run_ast_strong(ctx, c,s,e)));
			}
			
			auto strongCall(TransactionContext* ctx, StrongCache &c, const StrongStore &s, std::true_type*, std::false_type*, std::false_type*) const {
				run_ast_strong(ctx,c,s,t);
				run_ast_strong(ctx, c,s,e);
				//hndl.put(ctx->trackingContext->trk,ctx,Assignment::hndle_get(ctx,run_ast_strong(ctx, c,s,e)));
			}
			
			auto strongCall(TransactionContext* ctx, StrongCache &c, const StrongStore &s, std::true_type*, std::true_type*, std::false_type*) const {
				run_ast_strong(ctx,c,s,t);
				c.insert(id,Assignment::hndle_get(ctx,run_ast_strong(ctx, c,s,e)));
				//hndl.put(ctx->trackingContext->trk,ctx,Assignment::hndle_get(ctx,run_ast_strong(ctx, c,s,e)));
			}

			void strongCall(TransactionContext* ctx, StrongCache &c, const StrongStore &s, std::false_type*, std::false_type*, std::true_type*) const {
				static_assert(get_level<T>::value < 0, "Causal expression evaluates to strong handle; info-flow violation, cannot assign to handle");
			}

			bool causalCall(TransactionContext* ctx, CausalCache &c, const CausalStore &s) const {
				choose_strong<get_level<T>::value > choice1{nullptr};
				choose_strong<get_level<run_result<T> >::value> choice2{nullptr};
				causalCall(ctx, c,s,choice1,choice2);
				return true;
			}

			auto causalCall(TransactionContext* ctx, CausalCache &c, const CausalStore &s, std::false_type*,std::false_type*) const {
				if (runs_with_strong(get_level<Expr>::value) ){
					run_ast_causal(ctx,c,s,t).put(ctx->trackingContext->trk,ctx,c.get<HDref<Expr> >(id));
				}
				else {
					run_ast_causal(ctx,c,s,t).put(ctx->trackingContext->trk,ctx,Assignment::hndle_get(run_ast_causal(ctx, c,s,e)));
				}
			}

			auto causalCall(TransactionContext* ctx, CausalCache &c, const CausalStore &s, std::true_type*,std::true_type*) const {
				constexpr auto l = get_level<T>::value;
				static_assert(l == get_level<Expr>::value && runs_with_strong(l),"Error: assignment of strong to causal");
			}

			auto causalCall(TransactionContext* ctx, CausalCache &c, const CausalStore &s, std::true_type*,std::false_type*) const {
				if (runs_with_strong(get_level<Expr>::value) ){
					//t.causalCall(ctx,c,s).put(ctx->trackingContext->trk,ctx,c.get<HDref<Expr> >(id));
					run_ast_causal(ctx,c,s,t).put(ctx->trackingContext->trk,ctx,c.get<HDref<Expr> >(id));
				}
				else {
					//t.causalCall(ctx,c,s).put(ctx->trackingContext->trk,ctx,Assignment::hndle_get(run_ast_causal(ctx, c,s,e)));
					run_ast_causal(ctx,c,s,t).put(ctx->trackingContext->trk,ctx,Assignment::hndle_get(ctx,run_ast_causal(ctx, c,s,e)));
				}
			}
		};

		template<unsigned long long ID, Level l, typename T, HandleAccess HA, typename E>
		auto operator<<(const RefTemporary<ID,l,Handle<l,HA,T>,Temporary<ID,l,Handle<l,HA,T> > >& rt, const E &e){
			return Assignment<Handle<l,HA,T>,E>{rt.t.t,e};
		}

		template<unsigned long long ID, Level l1, Level l2, typename T, HandleAccess HA, typename E>
		auto operator<<(const RefTemporary<ID,l1,EnvironmentExpression<Handle<l2,HA,T> >,
						Temporary<ID,l1,EnvironmentExpression<Handle<l2,HA,T> > > >& rt,
						const E &e){
			return Assignment<EnvironmentExpression<Handle<l2,HA,T> >,E>{rt.t.t,e};
		}

		template<unsigned long long ID, typename T, typename Expr>
		auto find_usage(const Assignment<T,Expr> &rt){
			return find_usage<ID>(rt.e);
		}

	} }
