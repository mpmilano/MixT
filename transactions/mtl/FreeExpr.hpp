#pragma once
#include "utils.hpp"
#include "tuple_extras.hpp"
#include "type_utils.hpp"
#include "ConExpr.hpp"
#include "FreeExpr_macros.hpp"

namespace myria { namespace mtl {

template<typename T, Level l, HandleAccess ha>
auto get_if_handle(Handle<l,ha,T> h){
	return h.get();
}

template<typename T, restrict(!is_handle<T>::value)>
T get_if_handle(const T&t){
	return t;
}

template<unsigned long long id, Level l, typename T, typename Temp>
struct extract_type<RefTemporary<id,l,T,Temp> >{
	using type = typename
		extract_type<run_result<RefTemporary<id,l,T,Temp> > >::type;
};

template<unsigned long long ID, Level l, typename T, typename Temp, StoreType st>
void print_more_info_if_reftemp(const StoreMap<st>& c, const RefTemporary<ID,l,T,Temp> &rt){
	std::cerr << "ID of temporary referenced: " << ID << std::endl;
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
		std::cerr << "Type we failed on: " << type_name<E>() << std::endl;
		print_more_info_if_reftemp(c,e);
		return cached(c,e);
	}
}

template<typename T, typename... Exprs>
struct FreeExpr : public ConExpr<T, min_level_dref<Exprs...>::value > {

	//this one is just for temp-var-finding
	const std::tuple<Exprs...> params;
	using level = std::integral_constant<Level, min_level_dref<Exprs...>::value>;
	using Cache = std::conditional_t<runs_with_strong(level::value),StrongCache,CausalCache>;
	const std::function<T (const Cache&, const std::tuple<Exprs ...>& )> f;
	const int id = gensym();

	FreeExpr(int,
			 std::function<T (const typename extract_type<decay<Exprs> >::type & ... )> _f,
			 Exprs... h)
		:params(std::make_tuple(h...)),
		 f([_f](const Cache& c, const std::tuple<Exprs...> &t){
				 auto retrieved = fold(
					 t,
					 [&](const auto &e, const auto &acc){return std::tuple_cat(acc,std::make_tuple(get_if_handle(debug_failon_not_cached(c,e))));}
					 ,std::tuple<>());
				 static_assert(std::tuple_size<decltype(retrieved)>::value == sizeof...(Exprs),"You lost some arguments");
				 return callFunc(_f,retrieved);
			 })
		{
			static_assert(level::value == get_level<FreeExpr>::value, "Error: FreeExpr level determined inconsistently");
		}

	auto handles() const {
		return ::handles(params);
	}
	
	auto strongCall(StrongCache& cache, const StrongStore &heap) const{
		choose_strong<level::value> choice{nullptr};
		return strongCall(cache,heap,choice);
	}

	T strongCall(StrongCache& cache, const StrongStore &heap, std::true_type*) const{
		//everything is strong, run it now; but f assumes everything
		//already cached, which means strongCall for caching first
		std::false_type* false_t(nullptr);
		strongCall(cache,heap,false_t);
		auto ret = f(cache,params);
		assert(!cache.contains(this->id));
		cache.insert(this->id,ret);
		return ret;
	}

	void strongCall(StrongCache& cache, const StrongStore &heap,std::false_type*) const{
		foreach(params,[&](const auto &e){
				assert(!is_cached(cache,e) || is_handle<std::decay_t<decltype(e)> >::value);
				
				auto prev_ctx = context::current_context(cache);
				constexpr bool read_mode = is_handle<run_result<std::decay_t<decltype(e)> > >::value &&
					!is_preserve<std::decay_t<decltype(e)> >::value;
				constexpr bool data_mode = is_preserve<std::decay_t<decltype(e)> >::value;
				if (read_mode)
					context::set_context(cache,context::t::read);
				else if (data_mode)
					context::set_context(cache,context::t::data);
				
				run_ast_strong(cache,heap,e);
				
				if (read_mode || data_mode)
					context::set_context(cache,prev_ctx);

			});

		foreach(params,[&cache](const auto &e){
				assert(is_cached(cache,e));});
	}

	auto causalCall(CausalCache& cache, const CausalStore &heap) const {
		choose_causal<level::value> choice{nullptr};
		return causalCall(cache,heap,choice);
	}

	auto causalCall(CausalCache& cache, const CausalStore &heap, std::true_type*) const {
		fold(params,[&](const auto &e, bool){
				run_ast_causal(cache,heap,e);
				return false;},false);
		return f(cache,params);
	}

	T causalCall(CausalCache& cache, const CausalStore &heap, std::false_type*) const {
		assert(cache.contains(this->id));
		return cache.get<T>(this->id);
	}
	
	template<typename F>
	FreeExpr(F f, Exprs... h):FreeExpr(0, convert(f), h...){}
};

template<typename T, typename... H>
struct is_ConExpr<FreeExpr<T,H...> > : std::true_type {};

template<unsigned long long ID, typename T, typename... Vars>
auto find_usage(const FreeExpr<T,Vars...> &op){
	return fold(op.params,
				[](const auto &e, const auto &acc){
					return choose_non_np(acc,find_usage<ID>(e));
				}
				, nullptr);
}


	} }
