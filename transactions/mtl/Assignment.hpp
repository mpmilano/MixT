#pragma once
#include "Handle.hpp"
#include "EnvironmentExpression.hpp"
#include "RefTemporary.hpp"

namespace myria { namespace mtl {

                template<typename Expr, unsigned long long ID, Level l, typename T>
		struct Assignment : public ConStatement<get_level<Expr>::value>{
			template<typename T2>
			struct is_string : public std::is_same<T2, std::string> {};
	
		public:

                        using Temp = Temporary<ID,l,T>;
                        using RefTemp = RefTemporary<Temp>;

                        const Expr e;
                        const RefTemp temp;

                        static_assert(runs_with_causal(get_level<Expr>::value) ||
                                      (runs_with_strong(get_level<Expr>::value) && runs_with_strong(get_level<RefTemp>::value)),
                                      "Error: evaluation order won't work out.");

			const int id = mutils::gensym();
                        Assignment(const RefTemp &temp, const Expr & e)
                                :temp(temp),e(e)
				{
					static_assert(is_handle<run_result<T> >::value,"Error: Assignment construct only for modifying handles!");
					static_assert(
						can_flow(get_level<Expr>::value, get_level<run_result<T> >::value),
						"Error: assignment to strong member from causal expression!"
						);
					static_assert(
                                                can_flow(get_level<Expr>::value, get_level<T>::value),
                                                "Error: expression bound to temporary is strong; Expression attempted for handle update is causal"
						);
                                        static_assert(can_flow(get_level<Expr>::value, l),"Error: temporary was bound in strong context, we are now operating in causal context");
                                        static_assert(std::is_same<run_result<Expr>, typename run_result<T>::stored_type >::value, "Error: Assignment of incompatible types (no subtyping applies here)" );
				}

			auto environment_expressions() const{
                                return std::tuple_cat(mtl::environment_expressions(e),mtl::environment_expressions(temp));
			}

                        bool strongCall(std::true_type*, TransactionContext *ctx, StrongCache &c, StrongStore &s) const{
                            auto hndl = run_ast_strong(ctx,c,s,temp);
                            auto rhs = run_ast_strong(ctx,c,s,e);
                            hndl.put(ctx->trackingContext->trk,ctx,rhs);
                            return true;
                        }

                        bool strongCall(std::false_type*, TransactionContext *ctx, StrongCache &c, StrongStore &s) const{
                            run_ast_strong(ctx,c,s,temp);
                            run_ast_strong(ctx,c,s,e);
                            return true;
                        }

                        bool strongCall(TransactionContext *ctx, StrongCache &c, StrongStore &s) const {
                            choose_strong<get_level<Expr>::value > choice{nullptr};
                            return strongCall(choice,ctx,c,s);
                        }

                        bool causalCall(TransactionContext *ctx, CausalCache &c, CausalStore &s){
                            if (runs_with_strong(get_level<Expr>::value)) {
                                run_ast_causal(ctx,c,s,temp);
                                run_ast_strong(ctx,c,s,e);
                                return true; //we already did this during strong pass
                            }
                            else {
                                auto hndl = run_ast_causal(ctx,c,s,temp);
                                auto rhs = run_ast_causal(ctx,c,s,e);
                                hndl.put(ctx->trackingContext->trk,ctx,rhs);
                                return true;
                            }
                        }
		};

                template<typename Expr, unsigned long long ID, Level l, typename T>
                auto make_assignment(const RefTemporary<Temporary<ID,l,T> > &temp, const Expr &e){
                        return Assignment<Expr,ID,l,T>{temp,e};
		}
		


		template<unsigned long long ID, Level l1, typename T, typename E>
                auto operator<<(const RefTemporary<Temporary<ID,l1,T > >& rt,
						const E &e){
                        return make_assignment(rt,e);
		}

		namespace{
			template<typename T>
				auto& operator<<(T& os, std::nullptr_t){
				return os << "(nullpointer)";
			}
		}
		
                template<unsigned long long ID, typename Expr, unsigned long long ID2, Level l, typename T>
                auto find_usage(const Assignment<Expr,ID2,l,T> &rt){
                        return mutils::choose_non_np(find_usage<ID>(rt.temp),find_usage<ID>(rt.e));
		}

	} }
