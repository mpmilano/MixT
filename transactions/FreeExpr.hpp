#pragma once
#include "utils.hpp"
#include "tuple_extras.hpp"
#include "type_utils.hpp"
#include "ConExpr.hpp"

template<typename T, typename... Exprs>
struct FreeExpr : public ConExpr<T, min_level<Exprs...>::value > {

	//this one is just for temp-var-finding
	const std::tuple<Exprs...> params;
	const std::function<T (const Store&, const std::tuple<Exprs ...>& )> f;
	const BitSet<HandleAbbrev> rs;
	static constexpr Level level = min_level<Exprs...>::value;
	
	FreeExpr(int,
			 std::function<T (const typename extract_type<Exprs>::type & ... )> f,
			 Exprs... h)
		:params(std::make_tuple(h...)),
		 f([=](const Store &c, const std::tuple<Exprs...> &t){
				 auto retrieved = fold(t,
									   [&](const auto &e, const auto &acc){return tuple_cons(cached(c,e),acc);}
									   ,std::tuple<>());
				 return callFunc(f,retrieved);
			 }),
		 rs(setify(h.abbrev()...))
		{}

	auto strongCall(Store &cache, const Store &heap){
		std::integral_constant<bool,level==Level::strong>* choice = nullptr;
		return strongCall(cache,heap,choice);
	}	

	T strongCall(Store &cache, const Store &heap, std::true_type*) const{
		std::false_type* false_t(nullptr);
		strongCall(false_t);
		auto ret = f(cache,params);
		cache.insert(this->id,ret);
		return ret;
	}

	void strongCall(Store &cache, const Store &heap,std::false_type*) const{
		fold(params,[&](const auto &e, bool){
				e.strongCall(cache,heap);
				return false;},false);
	}

	auto causalCall(Store &cache, const Store &heap){
		std::integral_constant<bool,level==Level::causal>* choice = nullptr;
		return causalCall(cache,heap,choice);
	}	

	auto causalCall(Store &cache, const Store &heap, std::true_type*) const {
		fold(params,[&](const auto &e, bool){
				e.causalCall(cache,heap);
				return false;},false);
		return f(cache,params);
	}

	T causalCall(Store &cache, const Store &heap, std::false_type*) const {
		assert(cache.contains(this->id));
		return cache.get<T>(this->id);
	}

	T operator()(Store &c, const Store &s) const {
		if (level == Level::strong) return strongCall(c,s);
		else {
			strongCall(c,s);
			return causalCall(c,s);
		}
	}
	
	BitSet<HandleAbbrev> getReadSet() const {
		return *rs;
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
					if (!acc){
						return find_usage<ID>(e);
					}
					else return acc;
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
