#pragma once
#include "Operations.hpp"
#include "Handle.hpp"
#include "ConExpr.hpp"
#include "TempBuilder.hpp"

namespace myria{
	namespace mtl{

		template<typename...> struct FindUsages;

		template<RegisteredOperations Name, typename Handle, typename... Args>
		auto do_op_3(TransactionContext* ctx, typename SupportedOperation<Name,SelfType,Args...>::
					 template SupportsOn<Handle> &hndl,
					 typename convert_SelfType<Handle&>::template act<Args>... args){

			assert(hndl.op && "error! This is probably a null handle");
			return hndl.op->act(ctx,hndl.downCast(),args...);
		}
		
		template<RegisteredOperations Name, typename Handle, typename... Args>
		auto do_op_2(TransactionContext* ctx, Handle h, Args && ... args){
			constexpr OperationIdentifier<Name> op{nullptr};
			return do_op_3<Name,Handle,Args...>(ctx,h.upCast(op),std::forward<Args>(args)...);
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
		
		//all these arguments are shared_ptrs
		template<RegisteredOperations Name, typename... Args>
		auto do_op_sp(Args ... args){

			using strong_call_t
				= std::function<bool (TransactionContext* , StrongCache &, StrongStore &)>;
			
			using causal_call_t
				= std::function<bool (TransactionContext* , CausalCache &, CausalStore &)>;

			using FindUsage = FindUsages<de_sp<Args>...>;
			constexpr Level level = min_level_dref<de_sp<Args>...>::value;
			
			strong_call_t strongCall =
				(runs_with_strong(level) ?
				 //strong only
				 strong_call_t{[=](TransactionContext* ctx, StrongCache &c, StrongStore &s){
						assert(ctx);
						eat([&](){run_ast_strong(ctx,c,s,*args); return nullptr;}()...);
						ctx->template get_store_context<level>(first(cached(c,*args)...).store(),"Doing a strong operation");
						do_op_2<Name>(ctx,cached(c,*args)...);
						return true;
					}} :
				 //mixed
				 strong_call_t{[=](TransactionContext* ctx, StrongCache &c, StrongStore &s){
						 assert(ctx);
						 eat([&](){run_ast_strong(ctx,c,s,*args); return nullptr;}()...);
						 return true;
					 }});

			causal_call_t causalCall =
				(runs_with_strong(level) ?
				 //pretty much don't do anything
				 causal_call_t{[=](TransactionContext* ctx, CausalCache &c, CausalStore &s){
						assert(ctx);
						eat(run_ast_causal(ctx,c,s,*args)...);
						//nothing interesting in the cache to return, this is an operation
						return true;
					}} :
				 //mixed
				 causal_call_t{[=](TransactionContext* ctx, CausalCache &c, CausalStore &s){
						 assert(ctx);
						 eat(run_ast_causal(ctx,c,s,*args)...);
						 ctx->template get_store_context<level>(first(cached(c,*args)...).store(),"Doing a causal operation");
						 do_op_2<Name>(ctx,cached(c,*args)...);
						 return true;
					 }});

			auto env_exprs = std::tuple_cat(environment_expressions(*args)...);
			using env_exprs_t = decltype(env_exprs);
				 
			struct Operate :
				public FindUsage,
				public ConStatement<level>
				{

					using name = std::integral_constant<RegisteredOperations,Name>;
					const int id = mutils::gensym();

					const strong_call_t strongCall;
					const causal_call_t causalCall;
					const env_exprs_t env_exprs;

					env_exprs_t environment_expressions() const {
						return env_exprs;
					}
					
					Operate(strong_call_t strongCall,
							causal_call_t causalCall,
							env_exprs_t env_exprs,
							Args... args):
						FindUsage(*args...),
						strongCall(strongCall),
						causalCall(causalCall),
						env_exprs(env_exprs){}
				
				};
			return Operate{strongCall, causalCall, env_exprs, args...};
		}

		template<RegisteredOperations Name, typename... Args>
		auto do_op(Args && ... args){
			return do_op_sp<Name>(std::make_shared<std::decay_t<Args> >(args)...);
		}
	}
}
