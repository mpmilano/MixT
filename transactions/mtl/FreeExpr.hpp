#pragma once
#include "utils.hpp"
#include "tuple_extras.hpp"
#include "type_utils.hpp"
#include "ConExpr.hpp"
#include "RefTemporary.hpp"
#include "Preserve.hpp"
#include "FreeExpr_macros.hpp"

namespace myria { namespace mtl {

		template<typename T, Level l, HandleAccess ha,typename... Ops>
		auto get_if_handle(tracker::Tracker &trk, TransactionContext *ctx, Handle<l,ha,T,Ops...> h){
			assert(ctx);
			//TODO: eliminate copy?
			return *h.get(trk,ctx);
		}

		template<typename T, restrict(!is_handle<T>::value)>
		T get_if_handle(tracker::Tracker &trk, TransactionContext *ctx, const T&t){
			return t;
		}

		template<typename Temp>
		struct extract_type<RefTemporary<Temp> >{
			using type = typename
				extract_type<run_result<RefTemporary<Temp> > >::type;
		};

		template<typename Temp, StoreType st>
		void print_more_info_if_reftemp(const StoreMap<st>& c, const RefTemporary<Temp> &rt){
			std::cerr << "RefTemp ID referenced: " << rt.id << std::endl;
			std::cerr << "RefTemp name referenced: " << rt.name << std::endl;
			std::cerr << "address of cache: " << &c << std::endl;
		}

		template<typename T, StoreType st>
		void print_more_info_if_reftemp(const StoreMap<st> &, const T&){}

		template<typename C, typename E>
		auto debug_failon_not_cached(const C& c, const E &e){
			try {
				return cached(c,e);
			}
			catch (const CacheLookupFailure&){
				std::cerr << "found a failure point!" << std::endl;
				std::cerr << "Type we failed on: " << mutils::type_name<E>() << std::endl;
				print_more_info_if_reftemp(c,e);
				return cached(c,e);
			}
		}

		template<typename T, typename... Exprs> struct FreeExpr;

		template<typename T, typename... Exprs> struct FreeExpr<T&, Exprs...> : public FreeExpr<T,Exprs...>{
			template<typename F>
			FreeExpr(F f, Exprs... h):FreeExpr<T,Exprs...>(std::forward<F>(f),std::forward<Exprs>(h)...){}
		};
		template<typename T, typename... Exprs> struct FreeExpr<T&&, Exprs...> : public FreeExpr<T,Exprs...>{
			template<typename F>
				FreeExpr(F f, Exprs... h):FreeExpr<T,Exprs...>(std::forward<F>(f),std::forward<Exprs>(h)...){}
		};

		template<typename T, typename... Exprs>
		struct FreeExpr : public ConExpr<std::decay_t<T>, min_level_dref<Exprs...>::value > {

			//this one is just for temp-var-finding
			const std::tuple<Exprs...> params;
			using level = std::integral_constant<Level, min_level_dref<Exprs...>::value>;
			using Cache = std::conditional_t<runs_with_strong(level::value),StrongCache,CausalCache>;
			const std::function<T (TransactionContext *, const Cache&, const std::tuple<Exprs ...>& )> f;
			const int id = mutils::gensym();

			FreeExpr(int,
					 std::function<T (const typename extract_type<std::decay_t<Exprs> >::type & ... )> _f,
					 Exprs... h)
				:params(std::make_tuple(h...)),
				 f([_f](TransactionContext *ctx, const Cache& c, const std::tuple<Exprs...> &t){
						 auto retrieved = mutils::fold(
							 t,
							 [&](const auto &e, const auto &acc){
								 return std::tuple_cat(
									 acc,std::make_tuple(
										 get_if_handle(
											 ctx->trackingContext->trk, ctx,debug_failon_not_cached(c,e))));}
							 ,std::tuple<>());
						 static_assert(std::tuple_size<decltype(retrieved)>::value == sizeof...(Exprs),"You lost some arguments");
						 return mutils::callFunc(_f,retrieved);
					 })
				{
					static_assert(level::value == get_level<FreeExpr>::value, "Error: FreeExpr level determined inconsistently");
				}

			auto environment_expressions() const {
				return mtl::environment_expressions(params);
			}
	
			auto strongCall(TransactionContext *ctx, StrongCache& cache, const StrongStore &heap) const{
				choose_strong<level::value> choice{nullptr};
				return strongCall(ctx,cache,heap,choice);
			}

			std::decay_t<T&> strongCall(TransactionContext *ctx, StrongCache& cache, const StrongStore &heap, std::true_type*) const{
				//everything is strong, run it now; but f assumes everything
				//already cached, which means strongCall for caching first
				std::false_type* false_t(nullptr);
				strongCall(ctx,cache,heap,false_t);
				auto ret = f(ctx,cache,params);
				assert(!cache.contains(this->id));
				cache.insert(this->id,ret);
				return ret;
			}

			void strongCall(TransactionContext *ctx, StrongCache& cache, const StrongStore &heap,std::false_type*) const{
				mutils::foreach(params,[&](const auto &e){
						assert(!is_cached(cache,e) || is_handle<std::decay_t<decltype(e)> >::value);

						assert(ctx);
						auto prev_ctx = ctx->execution_context;
						constexpr bool read_mode = is_handle<run_result<std::decay_t<decltype(e)> > >::value &&
							!is_preserve<std::decay_t<decltype(e)> >::value;
						constexpr bool data_mode = is_preserve<std::decay_t<decltype(e)> >::value;
						if (read_mode)
							ctx->execution_context = context::t::read;
						else if (data_mode)
							ctx->execution_context = context::t::data;
						
						run_ast_strong(ctx,cache,heap,e);
				
						if (read_mode || data_mode)
							ctx->execution_context = prev_ctx;

					});

				mutils::foreach(params,[&cache](const auto &e){
						assert(is_cached(cache,e));});
			}

			auto causalCall(TransactionContext *ctx, CausalCache& cache, const CausalStore &heap) const {
				choose_causal<level::value> choice{nullptr};
				return causalCall(ctx,cache,heap,choice);
			}

			auto causalCall(TransactionContext *ctx, CausalCache& cache, const CausalStore &heap, std::true_type*) const {
				mutils::fold(params,[&](const auto &e, bool){
						run_ast_causal(ctx,cache,heap,e);
						return false;},false);
				return f(ctx,cache,params);
			}

			T causalCall(TransactionContext *ctx, CausalCache& cache, const CausalStore &heap, std::false_type*) const {
				assert(cache.contains(this->id));
				return cache.get<T>(this->id);
			}
	
			template<typename F>
			FreeExpr(F f, Exprs... h):FreeExpr(0, mutils::convert(f), h...){}
		};

		template<typename T, typename... H>
		struct is_ConExpr<FreeExpr<T,H...> > : std::true_type {};

		template<unsigned long long ID, typename T, typename... Vars>
		auto find_usage(const FreeExpr<T,Vars...> &op){
			return mutils::fold(op.params,
								[](const auto &e, const auto &acc){
									return mutils::choose_non_np(acc,find_usage<ID>(e));
								}
								, nullptr);
		}


	} }
