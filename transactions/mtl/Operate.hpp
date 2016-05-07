#pragma once
#include "Operations.hpp"
#include "Handle.hpp"
#include "ConExpr.hpp"
#include "TempBuilder.hpp"

namespace myria{
	namespace mtl{

		template<typename...> struct FindUsages;

		template<RegisteredOperations Name, typename Handle, typename Ret, typename... Args>
		auto do_op_3(TransactionContext* ctx, typename SupportedOperation<Name,Ret,SelfType,Args...>::
					template SupportsOn<Handle> &hndl,
					typename convert_SelfType<Handle&>::template act<Args>... args){

			assert(hndl.op && "error! This is probably a null handle");
			return hndl.op->act(ctx,hndl.downCast(),args...);
		}
		
		template<RegisteredOperations Name, typename Handle, typename... Args>
		auto do_op_2(TransactionContext* ctx, Handle h, Args && ... args){
			constexpr OperationIdentifier<Name> op{nullptr};
			using ret_t = typename std::decay_t<decltype(h.upCast(op))>::return_raw;
			return do_op_3<Name,Handle,ret_t,Args...>(ctx,h.upCast(op),std::forward<Args>(args)...);
		}

		template<typename> struct de_sp_str;

		template<typename T>
		struct de_sp_str<std::shared_ptr<T> >{
			using t = T;
		};

		template<typename T>
		using de_sp = typename de_sp_str<T>::t;

		template<typename... T>
		void eat(const T&...){}

		template<typename hd, typename... tl>
		auto first(const hd &h, const tl & ...){
			return h;
		}

		template<RegisteredOperations Name, typename... Args>
		using operate_result = decltype(do_op_2<Name>(
										   mutils::mke_p<TransactionContext>(),
										   cached(std::declval<CausalCache>(),
												  *std::declval<Args>())...));

		template<RegisteredOperations Name, typename... Args>
		struct Operate :
			public FindUsages<de_sp<Args>...>,
			public ConExpr<operate_result<Name,Args...>,min_level_dref<de_sp<Args>...>::value>
		{

			constexpr static Level level = min_level_dref<de_sp<Args>...>::value;

			const int id = mutils::gensym();
			
			using op_result = operate_result<Name,Args...>;
			
			using strong_call_t
			= std::function<std::unique_ptr<op_result> (int id, TransactionContext* , StrongCache &, const StrongStore &)>;
			
			using causal_call_t
			= std::function<std::unique_ptr<op_result> (int id, TransactionContext* , CausalCache &, const CausalStore &)>;

			using env_exprs_t = std::decay_t<decltype(std::tuple_cat(environment_expressions(*std::declval<Args>())...))>;
			
			using name = std::integral_constant<RegisteredOperations,Name>;
			
			const strong_call_t strongCall_;
			const causal_call_t causalCall_;

			void strongCall(TransactionContext* tc, StrongCache &c, const StrongStore &s, std::false_type*) const {
				strongCall_(id,tc,c,s);
			}

			op_result strongCall(TransactionContext* tc, StrongCache &c, const StrongStore &s, std::true_type*) const {
				return *strongCall_(id,tc,c,s);
			}

			auto strongCall(TransactionContext* tc, StrongCache &c, const StrongStore &s) const {
				choose_strong<level> choice{nullptr};
				return strongCall(tc,c,s,choice);
			}

			op_result causalCall(TransactionContext* tc, CausalCache &c, const CausalStore &s) const {
				return *causalCall_(id,tc,c,s);
			}
			
			const env_exprs_t env_exprs;

			env_exprs_t environment_expressions() const {
				return env_exprs;
			}
			
			Operate(strong_call_t strongCall,
					causal_call_t causalCall,
					env_exprs_t env_exprs,
					Args... args):
				FindUsages<de_sp<Args>...>(*args...),
				strongCall_(strongCall),
				causalCall_(causalCall),
				env_exprs(env_exprs){}
			
		};

		template<RegisteredOperations Name, typename... Args>
		struct is_ConExpr<Operate<Name,Args...> > : std::true_type {};

		
		//all these arguments are shared_ptrs
		template<RegisteredOperations Name, typename... Args>
		auto do_op_sp(Args ... args){

			using Operate = Operate<Name,Args...>;
			using op_result = typename Operate::op_result;
			using strong_call_t = typename Operate::strong_call_t;
			using causal_call_t = typename Operate::causal_call_t;
			
			strong_call_t strongCall =
				(runs_with_strong(Operate::level) ?
				 //strong only
				 strong_call_t{[=](int id, TransactionContext* ctx, StrongCache &c, const StrongStore &s) {
						assert(ctx);
						eat([&](){run_ast_strong(ctx,c,s,*args); return nullptr;}()...);
						ctx->template get_store_context<Operate::level>(first(cached(c,*args)...).store(),"Doing a strong operation");
						auto ret = do_op_2<Name>(ctx,cached(c,*args)...);
						c.insert(id,ret);
						return mutils::heap_copy(ret);
					}} :
				 //mixed
				 strong_call_t{[=](int, TransactionContext* ctx, StrongCache &c, const StrongStore &s) {
						 assert(ctx);
						 eat([&](){run_ast_strong(ctx,c,s,*args); return nullptr;}()...);
						 return std::unique_ptr<op_result>{nullptr};
					 }});

			causal_call_t causalCall =
				(runs_with_strong(Operate::level) ?
				 //retrieve previously-calculated value
				 causal_call_t{[=](int id, TransactionContext* ctx, CausalCache &c, const CausalStore &s) {
						assert(ctx);
						eat(run_ast_causal(ctx,c,s,*args)...);
						return mutils::heap_copy(c.template get<op_result>(id));
					}} :
				 //mixed
				 causal_call_t{[=](int id, TransactionContext* ctx, CausalCache &c, const CausalStore &s) {
						 assert(ctx);
						 eat(run_ast_causal(ctx,c,s,*args)...);
						 ctx->template get_store_context<Operate::level>(first(cached(c,*args)...).store(),"Doing a causal operation");
						 auto ret = do_op_2<Name>(ctx,cached(c,*args)...);
						 c.insert(id,ret);
						 return mutils::heap_copy(ret);
					 }});

			auto env_exprs = std::tuple_cat(environment_expressions(*args)...);
			return Operate{strongCall, causalCall, env_exprs, args...};
		}

		template<RegisteredOperations Name, typename... Args>
		auto do_op(Args && ... args){
			return do_op_sp<Name>(std::make_shared<std::decay_t<Args> >(args)...);
		}
	}
}
