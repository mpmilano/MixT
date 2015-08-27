#pragma once
#include "utils.hpp"
#include "tuple_extras.hpp"
#include "type_utils.hpp"
#include "ConExpr.hpp"
#include "FreeExpr_macros.hpp"

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

template<unsigned long long ID, Level l, typename T, typename Temp>
void print_more_info_if_reftemp(const Store& c, const RefTemporary<ID,l,T,Temp> &rt){
	std::cout << "ID of temporary referenced: " << ID << std::endl;
	std::cout << "RefTemp ID referenced: " << rt.id << std::endl;
	std::cout << "RefTemp name referenced: " << rt.name << std::endl;
	std::cout << "address of cache: " << &c << std::endl;
}

template<typename T>
void print_more_info_if_reftemp(const Store &, const T&){}

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
struct FreeExpr : public ConExpr<T, min_level<Exprs...>::value > {

	//this one is just for temp-var-finding
	const std::tuple<Exprs...> params;
	const std::function<T (const Store&, const std::tuple<Exprs ...>& )> f;
	using level = std::integral_constant<Level, min_level<Exprs...>::value>;
	const int id = gensym();

	FreeExpr(int,
			 std::function<T (const typename extract_type<decay<Exprs> >::type & ... )> f,
			 Exprs... h)
		:params(std::make_tuple(h...)),
		 f([=](const Store &c, const std::tuple<Exprs...> &t){
				 auto retrieved = fold(
					 t,
					 [&](const auto &e, const auto &acc){return tuple_cons(get_if_handle(debug_failon_not_cached(c,e)),acc);}
					 ,std::tuple<>());
				 return callFunc(f,retrieved);
			 })
		{}

	auto strongCall(Store &cache, const Store &heap) const{
		std::cout << "strong call" << std::endl;
		std::integral_constant<bool,level::value==Level::strong>* choice = nullptr;
		return strongCall(cache,heap,choice);
	}	

	T strongCall(Store &cache, const Store &heap, std::true_type*) const{
		//everything is strong, run it now; but f assumes everything
		//already cached, which means strongCall for caching first
		std::false_type* false_t(nullptr);
		strongCall(cache,heap,false_t);
		auto ret = f(cache,params);
		cache.insert(this->id,ret);
		return ret;
	}

	void strongCall(Store &cache, const Store &heap,std::false_type*) const{
		fold(params,[&](const auto &e, bool){
				run_ast_strong(cache,heap,e);
				return false;},false);

		fold(params,[&cache](const auto &e, bool){
				assert(is_cached(cache,e));
				return false;},false);
	}

	auto causalCall(Store &cache, const Store &heap) const {
		std::cout << "causal call" << std::endl;
		std::integral_constant<bool,level::value==Level::causal>* choice = nullptr;
		return causalCall(cache,heap,choice);
	}	

	auto causalCall(Store &cache, const Store &heap, std::true_type*) const {
		fold(params,[&](const auto &e, bool){
				run_ast_causal(cache,heap,e);
				return false;},false);
		return f(cache,params);
	}

	T causalCall(Store &cache, const Store &heap, std::false_type*) const {
		assert(cache.contains(this->id));
		return cache.get<T>(this->id);
	}
	
	BitSet<HandleAbbrev> getReadSet() const {
		assert(false && "stop using this");
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

template<unsigned long long ID, typename T, typename... Exprs>
struct contains_temporary<ID, FreeExpr<T,Exprs...> > : contains_temp_fold<ID,std::tuple<Exprs...> > {};

