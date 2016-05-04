#pragma once
#include "utils.hpp"
#include "tuple_extras.hpp"
#include "type_utils.hpp"
#include "ConExpr.hpp"
#include "RefTemporary.hpp"
#include "Preserve.hpp"
#include "FreeExpr_macros.hpp"

namespace myria { namespace mtl {

		template<typename T, typename R>
		struct FieldRef : public ConExpr<R,get_level<T>::value>{
			const T t;
			using FieldRefFun = std::function<R (const run_result<T>&)>;
			const FieldRefFun fun;
			const int id = mutils::gensym();

			FieldRef(const T &t, const FieldRefFun &fun):t(t),fun(fun){}

			auto environment_expressions() const {
				return mtl::environment_expressions(t);
			}

			auto strongCall(TransactionContext *ctx, StrongCache& c, const StrongStore &s,std::true_type*) const {
				auto ret = fun(run_ast_strong(ctx,c,s,t));
				c.insert(id,ret);
				return ret;
			}

			auto strongCall(TransactionContext *ctx, StrongCache &c, const StrongStore &s, std::false_type*) const {
				run_ast_strong(ctx,c,s,t);
			}

			auto strongCall(TransactionContext* ctx, StrongCache& c, const StrongStore& s) const {
				choose_strong<get_level<T>::value > choice{nullptr};
				return strongCall(ctx,c,s,choice);
			}

			R causalCall(TransactionContext *ctx, CausalCache& c, const CausalStore& s) const {
				if (c.contains(id)) return c.template get<R>(id);
				else {
					R ret = fun(run_ast_causal(ctx,c,s,t));
					c.insert(id,ret);
					return ret;
				}
			}
		};

		template<typename T, typename R> struct is_ConExpr<FieldRef<T,R> > : std::true_type {};

		template<unsigned long long ID, typename T, typename R>
		struct contains_temporary<ID,FieldRef<T,R> > : contains_temporary<ID,T> {};

		template<unsigned long long ID, typename T, typename R>
		auto find_usage(const FieldRef<T,R>& fr){
			return find_usage<ID>(fr.t);
		}

		template<typename T, typename R>
		auto make_fieldref(const T& t, const std::function<R (const run_result<T>&)> &f){
			return FieldRef<T,R>{t,f};
		}

                template<typename T, typename... MTLArgs>
                struct MTLCtr : public ConExpr<T,min_level<MTLArgs...>::value> {

                    using level = min_level<MTLArgs...>;

                    std::tuple<MTLArgs...> args;
                    const int id = mutils::gensym();

                    MTLCtr(const MTLArgs & ... args):args(std::make_tuple(args)){}

                    auto environment_expressions() const {
                        mutils::fold(args,
                                     [](const auto& arg, const auto& accum){
                            return std::tuple_cat(environment_expressions(arg),accum);
                        },std::tuple<>());
                    }

                    T strongCall(TransactionContext *ctx, StrongCache &c, const StrongStore &s, std::true_type*){
                        T (*builder) (const MTLArgs & ...) =
                                [&](const MTLArgs & ... args){return T{run_ast_strong(ctx,c,s,args)...};};
                        auto ret = callFunc(builder,args);
                        c.insert(this->id,ret);
                        return ret;
                    }
                    void strongCall(TransactionContext *ctx, StrongCache &c, const StrongStore &s, std::false_type*){
                        T (*callAll) (const MTLArgs & ...) =
                                [&](const MTLArgs & ... args){run_ast_strong(ctx,c,s,args);};
                        callFunc(callAll,args);
                    }
                    auto strongCall(TransactionContext *ctx, StrongCache &c, const StrongStore &s){
                        choose_strong<level::value> choice{nullptr};
                        return strongCall(ctx,c,s,choice);
                    }

                    T causalCall(TransactionContext *ctx, StrongCache &c, const StrongStore &s){
                        if (level::value == Level::strong){
                            return c.template get<T>(this->id);
                        }
                        else {
                            T (*builder) (const MTLArgs & ...) =
                                    [&](const MTLArgs & ... args){return T{run_ast_causal(ctx,c,s,args)...};};
                            T ret = callFunc(builder,args);
                            c.insert(this->id,ret);
                            return ret;
                        }
                    }
                };

                template<typename T, typename... Args> struct is_ConExpr<MTLCtr<T,Args...> > : std::true_type {};

                template<unsigned long long ID, typename T, typename... Args>
                struct contains_temporary<ID,MTLCtr<T,Args...> > :
                        std::integral_constant<bool,exists(contains_temporary<ID,Args>::value...)> {};

                template<unsigned long long ID, typename T, typename... Args>
                auto find_usage(const MTLCtr<T,Args...>& ctr){
                    return fold(ctr.args,[](const auto &arg, const auto &accum){
                        return choose_non_np(find_usage<ID>(arg),accum);
                    },nullptr);
                }

                template<typename T, typename... Args>
                auto make_mtl_ctr(const Args & ... args){
                    return MTLCtr<T,Args...>{args...};
                }

	} }
