#pragma once


#include "utils.hpp"
#include <string>
#include <memory>
#include <tuple>
#include "ConStatement.hpp"
#include "args-finder.hpp"

namespace myria { namespace mtl {

		typedef Level Level;

		template<typename T, Level l>
		struct ConExpr {
	
			//typename std::conditional<l == Level::strong, T, void>::type
			//virtual strongCall(Cache&, const Store&) const = 0;
			//virtual T causalCall(Cache&, const Store&) const = 0;
		};


		template<typename T, restrict(! (is_ConStatement<T>::value || is_handle<T>::value || mutils::is_tuple<T>::value || (is_ConExpr<T>::value && !std::is_scalar<T>::value)))>
		constexpr Level get_level_f(const T*){
			return Level::undef;
		}

		template<typename T, Level l>
		constexpr Level get_level_f(const ConExpr<T,l>*){
			return l;
		}

		//imported from ConStatement. Probably should get its own file or something.
		template<typename T>
		struct get_level : std::integral_constant<Level, get_level_f(mutils::mke_p<T>())>::type {};

		template<typename T, Level l>
		constexpr Level chld_min_level_f(ConExpr<T,l> const * const){
			return l;
		}

		template<typename T, restrict(std::is_scalar<T>::value)>
		constexpr Level chld_min_level_f(T const * const t){
			return get_level_f(t);
		}

		template<Level l>
		constexpr Level chld_min_level_f(level_constant<l> const * const ){
			return l;
		}

		template<typename T>
		struct chld_min_level : level_constant<chld_min_level_f(mutils::mke_p<T>())> {};

		template<typename T>
		struct chld_min_level<const T> : chld_min_level<T> {};

		template<typename... T>
		struct min_level : std::integral_constant<Level,
												  (exists((chld_min_level<T>::value == Level::causal)...) ?
												   Level::causal :
												   (exists((chld_min_level<T>::value == Level::strong)...) ? Level::strong : Level::undef )
													  )>::type {};

		template<typename T, restrict(std::is_scalar<T>::value)>
		constexpr Level get_level_dref(T*){
			return Level::undef;
		}

		template<HandleAccess ha, Level l, typename T,typename... Ops>
		constexpr Level get_level_dref(Handle<l,ha,T,Ops...> const * const){
			return l;
		}

		template<typename T, Level l, restrict2(is_handle<T>::value || is_ConExpr<T>::value)>
		constexpr Level get_level_dref(ConExpr<T,l> *){
			return (l == Level::causal ? Level::causal : get_level_dref(mutils::mke_p<T>()));
		}

		//if we've hit "Bottom" and can't continue to find level-having
		//items below this, then just return this level.
		template<typename T, Level l>
		constexpr std::enable_if_t<!is_handle<T>::value && !is_ConExpr<T>::value, Level>
			get_level_dref(ConExpr<T,l> *){
			return l;
		}


		template<typename... T>
		struct min_level_dref : std::integral_constant<Level,
													   exists((get_level_dref(mutils::mke_p<T>()) == Level::causal)...) ? Level::causal :
								(exists((get_level_dref(mutils::mke_p<T>()) == Level::strong)...) ? Level::strong :
								 Level::undef)
								> {
			static_assert(!exists((is_ConStatement<T>::value)...),"Error: min_level_dref only ready for Exprs. Not sure why you need this for non-exprs...");
		};

		template<typename... T>
		struct max_level_dref : std::integral_constant<Level,
													   exists((get_level_dref(mutils::mke_p<T>()) == Level::strong)...) ? Level::strong :
								(exists((get_level_dref(mutils::mke_p<T>()) == Level::causal)...) ? Level::causal :
								 Level::undef)
								> {
			static_assert(!exists((is_ConStatement<T>::value)...),"Error: max_level_dref only ready for Exprs. Not sure why you need this for non-exprs...");
		};

		template<typename... T>
		struct min_level<std::tuple<T...> > : min_level<T...> {};

		template<typename T>
		struct chld_max_level : get_level<T>{};

		template<typename T>
		struct chld_max_level<const T> : chld_max_level<T> {};

		template<typename... T>
		struct max_level : std::integral_constant<Level,
												  (exists((chld_max_level<T>::value == Level::strong)...) ?
												   Level::strong :
												   (exists((chld_max_level<T>::value == Level::causal)...) ?
													Level::causal : Level::undef))>::type {};

		template<typename... T>
		struct max_level<std::tuple<T...> > : max_level<T...> {};


		template<Level l>
		struct DummyConExpr : public ConExpr<void,l> {

			std::tuple<> environment_expressions() const {
				return std::tuple<>{};
			}
	
			void strongCall(TransactionContext*, const StrongCache&, const StrongStore &) const {}

			void causalCall(TransactionContext*, const CausalCache&, const CausalStore &) const {}	

		};

		template<typename T, Level l>
		constexpr bool is_ConExpr_f(ConExpr<T,l> const * const){
			return true;
		}

		template<typename T>
		constexpr bool is_ConExpr_f(const T*){
			return false;
		}

		template<typename Cls>
		struct is_ConExpr : 
		std::integral_constant<bool, is_ConExpr_f(mutils::mke_p<Cls>()) || std::is_scalar<std::decay_t<Cls> >::value>::type {};

		template<typename T, restrict(is_ConExpr<T>::value && !std::is_scalar<T>::value && !is_handle<T>::value)>
		auto run_ast_strong(TransactionContext* ctx, StrongCache& c, const StrongStore &s, const T& expr) {
			return expr.strongCall(ctx,c,s);
		}

		template<typename T>
		typename std::enable_if<std::is_scalar<std::decay_t<T > >::value,T>::type
		run_ast_strong(TransactionContext*, const StrongCache &, const StrongStore&, const T& e) {
			return e;
		}

		template<HandleAccess ha, typename T,typename... Ops>
		void run_ast_strong(TransactionContext *, const StrongCache& , const StrongStore&, const Handle<Level::causal,ha,T,Ops...>&) {
		}
		//*/


		std::string run_ast_strong(TransactionContext *ctx, const StrongCache &, const StrongStore&, const std::string& e);

		std::string run_ast_causal(TransactionContext *ctx, const CausalCache &, const CausalStore&, const std::string& e);


		template<HandleAccess ha, typename T,typename... Ops>
		Handle<Level::causal,ha,T,Ops...> run_ast_causal(TransactionContext *, const CausalCache& , const CausalStore &, const Handle<Level::causal,ha,T,Ops...>& t) {
			return t;
		}
		//*/

		template<typename T, restrict(is_ConExpr<T>::value &&
									  !std::is_scalar<T>::value
									  && !is_handle<T>::value)>
		auto run_ast_causal(TransactionContext *ctx, CausalCache& c, const CausalStore &s, const T& expr) {
			return expr.causalCall(ctx,c,s);
		}

		template<typename T>
		typename std::enable_if<std::is_scalar<std::decay_t<T > >::value,T>::type
		run_ast_causal(TransactionContext *, const CausalCache &, const CausalStore&, const T& e) {
			return e;
		}

		template<typename T>
		using run_result = decltype(run_ast_causal(std::declval<TransactionContext*>(), std::declval<CausalCache&>(),std::declval<CausalStore&>(),std::declval<T&>()));

		struct CacheLookupFailure : public mutils::StaticMyriaException<MACRO_GET_STR("Error: cache lookup failed!")> {};

		template<typename T, restrict(is_ConExpr<T>::value && !std::is_scalar<T>::value && !is_handle<T>::value)>
		auto cached(const StrongCache& cache, const T& ast){
			//TODO: make sure ast.id is always the gensym'd id.
			if (!cache.contains(ast.id)) throw CacheLookupFailure();
			using R = run_result<T>;
			return cache.get<R>(ast.id);
		}

		template<typename T, restrict(is_ConExpr<T>::value && !std::is_scalar<T>::value && !is_handle<T>::value)>
		auto cached(const CausalCache& cache, const T& ast){
			//TODO: make sure ast.id is always the gensym'd id.
			if (!cache.contains(ast.id)) throw CacheLookupFailure();
			using R = run_result<T>;
			return cache.get<R>(ast.id);
		}

		template<typename T, StoreType st>
		mutils::type_check<std::is_scalar, T> cached(const StoreMap<st>&, const T& e){
			return e;
		}


		template<typename T, HandleAccess ha, Level l, StoreType st,typename... Ops>
		Handle<l,ha,T,Ops...> cached(const StoreMap<st>&, const Handle<l,ha,T,Ops...>& ast){
			return ast;
		}

		template<typename T, restrict(is_ConExpr<T>::value && !std::is_scalar<T>::value && !is_handle<T>::value)>
		auto is_cached(const CausalCache& cache, const T& ast){
			return cache.contains(ast.id);
		}

		template<typename T, restrict(is_ConExpr<T>::value && !std::is_scalar<T>::value && !is_handle<T>::value)>
		auto is_cached(const StrongCache& cache, const T& ast){
			return cache.contains(ast.id);
		}

		template<typename T, StoreType st>
		std::enable_if_t<std::is_scalar<T>::value, bool> is_cached(const StoreMap<st>&, const T& ){
			return true;
		}


		template<typename T, HandleAccess ha, Level l, StoreType st,typename... Ops>
		bool is_cached(const StoreMap<st>& , const Handle<l,ha,T,Ops...>&){
			return true;
		}


		template<unsigned long long id, typename T>
		std::enable_if_t<std::is_scalar<T>::value || std::is_array<T>::value, std::nullptr_t> find_usage(const T&){
			return nullptr;
		}

		template<unsigned long long id>
		std::nullptr_t find_usage(const std::string&){
			return nullptr;
		}

		/*
		  template<unsigned long long id, typename T>
		  struct contains_temporary : std::false_type {
		  static_assert(std::is_scalar<T>::value,"Error: you apparently didn't finish defining enough contains_temporaries");
		  };
		*/

		template<unsigned long long id>
		struct contains_temporary<id,int> : std::false_type {};

		template<unsigned long long id>
		struct contains_temporary<id,bool> : std::false_type {};

		template<typename T, restrict(std::is_scalar<T>::value)>
		T extract_type_f(T *t){
			assert(false);
			struct dead_code{}; throw dead_code{};
			return *t;
		}

		template<typename T>
		struct extract_type {
			using type = decltype(extract_type_f(mutils::mke_p<T>()));
		};

	} }
