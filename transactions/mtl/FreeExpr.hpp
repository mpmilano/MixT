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

			auto strongCall(TransactionContext *ctx, StrongCache& c, StrongStore &s,std::true_type*) const {
				auto ret = fun(run_ast_strong(ctx,c,s,t));
				cache.insert(id,ret);
				return ret;
			}

			auto strongCall(TransactionContext *ctx, StrongCache &c, StrongStore &s, std::false_type*) const {
				run_ast_strong(ctx,c,s,t);
			}

			auto strongCall(TransactionContext* ctx, StrongCache& c, StrongStore& s) const {
				choose_strong<get_level<T> > choice{nullptr};
				return strongCall(ctx,c,s,choice);
			}

			R causalCall(TransactionContext *ctx, CausalCache& c, CausalStore& s) const {
				if (c.contains(id)) return c.template get<R>(id);
				else {
					R ret = fun(run_ast_causal(ctx,c,s,t));
					cache.insert(id,ret);
					return ret;
				}
			}
		};

		template<typename T, typename R> struct is_ConExpr<FieldRef<T,T> > : std::true_type {};

		template<unsigned long long ID, typename T, typename R>
		struct contains_temporary<ID,FieldRef<T,R> > : contains_temporary<ID,T> {};

		template<unsigned long long ID, typename T, typename R>
		auto find_usage(const FieldRef<T,R>& fr){
			return find_usage<ID>(fr.t);
		}

		template<typename T, typename R>
		make_fieldref(const T& t, const std::function<R (const run_result<T>&)> &f){
			return FieldRef<T,R>{t,r};
		}

	} }
