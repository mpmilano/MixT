#pragma once
#include "Operations.hpp"
#include "Handle.hpp"
#include "ConExpr.hpp"
#include "TempBuilder.hpp"

namespace myria{
	namespace mtl{

		template<typename...> struct FindUsages;

		template<RegisteredOperations Name, typename Handle, typename... Args>
		auto do_op_3(typename SupportedOperation<Name,SelfType,Args...>::
					 template SupportsOn<Handle> &hndl,
					 typename convert_SelfType<Handle&>::template act<Args>... args){
			
			return hndl.op->act(hndl.downCast(),args...);
		}
		
		template<RegisteredOperations Name, typename Handle, typename... Args>
		auto do_op_2(Handle &h, Args && ... args){
			constexpr OperationIdentifier<Name> op{nullptr};
			return do_op_3<Name,Handle>(h.upCast(op),std::forward<Args>(args)...);
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
		
		//all these arguments are shared_ptrs
		template<RegisteredOperations Name, typename... Args>
		auto do_op_sp(Args && ... args){

			using call_t = std::function<void (TransactionContext* , StrongCache &, StrongStore &)>;

			using FindUsage = FindUsages<de_sp<Args>...>;
			constexpr Level level = min_level_dref<de_sp<Args>...>::value;
			
			call_t strongCall =
				(runs_with_strong(level) ?
				 //strong only
				 [=](TransactionContext* ctx, StrongCache &c, StrongStore &s){
					eat(run_ast_strong(ctx,c,s,*args)...);
					return do_op_2<Name>(cached(*args)...);
				} :
				 //mixed
				 [=](TransactionContext* ctx, StrongCache &c, StrongStore &s){
					 eat(run_ast_strong(ctx,c,s,*args)...);
				 });

			call_t causalCall =
				(runs_with_strong(level) ?
				 //pretty much don't do anything
				 [=](TransactionContext* ctx, CausalCache &c, CausalStore &s){
					eat(run_ast_causal(ctx,c,s,*args)...);
					//nothing interesting in the cache to return, this is an operation not an expression
				} :
				 //mixed
				 [=](TransactionContext* ctx, CausalCache &c, CausalStore &s){
					 eat(run_ast_causal(ctx,c,s,*args)...);
					 return do_op_2<Name>(cached(*args)...);
				 });

			auto env_exprs = std::tuple_cat(environment_expressions(*args)...);
			using env_exprs_t = decltype(env_exprs);
				 
			struct Operate :
				public FindUsage,
				public ConStatement<level>
				{
					
					const int id = mutils::gensym();

					const call_t strongCall;
					const call_t causalCall;
					const env_exprs_t env_exprs;

					env_exprs_t environment_expressions() const {
						return env_exprs;
					}
					
					Operate(call_t strongCall,
							call_t causalCall,
							env_exprs_t env_exprs,
							Args... args):
						FindUsage(*args...),
						strongCall(strongCall),
						causalCall(causalCall),
						env_exprs(env_exprs){}
				
				};
			return Operate{strongCall, causalCall, args...};
		}
	}
}
