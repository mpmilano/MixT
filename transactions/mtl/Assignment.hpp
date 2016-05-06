#pragma once
#include "Handle.hpp"
#include "EnvironmentExpression.hpp"
#include "RefTemporary.hpp"

namespace myria { namespace mtl {

		template<typename Expr, typename T>
		struct Assignment : public ConStatement<get_level<Expr>::value>{
			template<typename T2>
			struct is_string : public std::is_same<T2, std::string> {};
	
		public:

			const Expr e;
			const T lhs;

			static_assert(runs_with_causal(get_level<Expr>::value) ||
						  (runs_with_strong(get_level<Expr>::value) && runs_with_strong(get_level<T>::value)),
						  "Error: evaluation order won't work out.");

			const int id = mutils::gensym();
			Assignment(const T &t, const Expr & e)
				:lhs(t),e(e)
				{
					static_assert(is_handle<run_result<T> >::value,"Error: Assignment construct only for modifying handles!");
					static_assert(
						can_flow(get_level<Expr>::value, get_level<run_result<T> >::value),
						"Error: assignment to strong member from causal expression!"
						);
					static_assert(
						can_flow(get_level<Expr>::value, get_level<T>::value),
						"Error: lhs expressionis strong; Expression attempted for handle update is causal"
						);
					static_assert(std::is_same<run_result<Expr>, typename run_result<T>::stored_type >::value, "Error: Assignment of incompatible types (no subtyping applies here)" );
				}

			auto environment_expressions() const{
				return std::tuple_cat(mtl::environment_expressions(e),mtl::environment_expressions(lhs));
			}

			bool strongCall(std::true_type*, TransactionContext *ctx, StrongCache &c, StrongStore &s) const{
				auto hndl = run_ast_strong(ctx,c,s,lhs);
				auto rhs = run_ast_strong(ctx,c,s,e);
				hndl.put(ctx->trackingContext->trk,ctx,rhs);
				return true;
			}

			bool strongCall(std::false_type*, TransactionContext *ctx, StrongCache &c, StrongStore &s) const{
				run_ast_strong(ctx,c,s,lhs);
				run_ast_strong(ctx,c,s,e);
				return true;
			}

			bool strongCall(TransactionContext *ctx, StrongCache &c, StrongStore &s) const {
				choose_strong<get_level<Expr>::value > choice{nullptr};
				return strongCall(choice,ctx,c,s);
			}

			bool causalCall(TransactionContext *ctx, CausalCache &c, CausalStore &s){
				if (runs_with_strong(get_level<Expr>::value)) {
					run_ast_causal(ctx,c,s,lhs);
					run_ast_strong(ctx,c,s,e);
					return true; //we already did this during strong pass
				}
				else {
					auto hndl = run_ast_causal(ctx,c,s,lhs);
					auto rhs = run_ast_causal(ctx,c,s,e);
					hndl.put(ctx->trackingContext->trk,ctx,rhs);
					return true;
				}
			}
		};

		template<typename Expr, typename T>
		auto make_assignment(const T &t, const Expr &e){
			return Assignment<Expr,T>{t,e};
		}


		template<unsigned long long ID, Level l1, typename T, typename E>
		auto operator<<(const RefTemporary<Temporary<ID,l1,T > >& rt,
						const E &e){
			return make_assignment(rt,e);
		}

		template<typename T, typename R, typename E>
		auto operator<<(const FieldRef<T,R>& fr,const E &e){
			return make_assignment(fr,e);
		}
		

		namespace{
			template<typename T>
				auto& operator<<(T& os, std::nullptr_t){
				return os << "(nullpointer)";
			}
		}
		
		template<unsigned long long ID, typename Expr, typename T>
		auto find_usage(const Assignment<Expr,T> &rt){
			return mutils::choose_non_np(find_usage<ID>(rt.lhs),find_usage<ID>(rt.e));
		}

	} }
