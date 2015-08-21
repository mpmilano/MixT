#pragma once
#include "utils.hpp"
#include "tuple_extras.hpp"
#include "type_utils.hpp"
#include "ConExpr.hpp"

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
		extract_type<decltype(mke<RefTemporary<id,l,T,Temp> >().
							  causalCall(mke_store(), mke_store()))>::type;
};

template<typename T, typename... Exprs>
struct FreeExpr : public ConExpr<T, min_level<Exprs...>::value > {

	//this one is just for temp-var-finding
	const std::tuple<Exprs...> params;
	const std::function<T (const Store&, const std::tuple<Exprs ...>& )> f;
	static constexpr Level level = min_level<Exprs...>::value;
	
	FreeExpr(int,
			 std::function<T (const typename extract_type<decay<Exprs> >::type & ... )> f,
			 Exprs... h)
		:params(std::make_tuple(h...)),
		 f([=](const Store &c, const std::tuple<Exprs...> &t){
				 auto retrieved = fold(t,
									   [&](const auto &e, const auto &acc){return tuple_cons(get_if_handle(cached(c,e)),acc);}
									   ,std::tuple<>());
				 return callFunc(f,retrieved);
			 })
		{}

	auto strongCall(Store &cache, const Store &heap) const{
		std::cout << "strong call" << std::endl;
		std::integral_constant<bool,level==Level::strong>* choice = nullptr;
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
	}

	auto causalCall(Store &cache, const Store &heap) const {
		std::cout << "causal call" << std::endl;
		std::integral_constant<bool,level==Level::causal>* choice = nullptr;
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

template<typename i, typename... E>
std::ostream & operator<<(std::ostream &os, const FreeExpr<i,E...>& op){
	return os << " apparently you can't print functions ";
}


#define free_expr3(T,a,e) (FreeExpr<T,decltype(a)>([&](const typename extract_type<decltype(a)>::type &a){return e;},a))
#define free_expr4(T,a,b,e) (FreeExpr<T,decltype(a),decltype(b)>([&](const typename extract_type<decltype(a)>::type &a, \
																	 const typename extract_type<decltype(b)>::type &b){return e;},a,b))


#define free_expr_IMPL2(count, ...) free_expr ## count (__VA_ARGS__)
#define free_expr_IMPL(count, ...) free_expr_IMPL2(count, __VA_ARGS__)
#define free_expr(...) free_expr_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__)

#define msg(a,b) free_expr(decltype(run_ast_causal(mke_store(),mke_store(),a).get().b), a, a.b)
