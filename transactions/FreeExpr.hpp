#pragma once
#include "utils.hpp"
#include "tuple_extras.hpp"
#include "type_utils.hpp"
#include "ConExpr.hpp"

template<typename T, typename... Handles>
struct FreeExpr : public ConExpr<T, min_level<Handles...>::value > {

	//todo: idea here is that only read-only things can be done to the handles
	//in this context.  Try to make that a reality please.
	const std::shared_ptr<const std::tuple<Handles...> > exprs;
	const std::shared_ptr<const std::function<T ()> > f;
	const std::shared_ptr<const BitSet<HandleAbbrev> > rs;
	
	FreeExpr(int,
			 std::function<T (const typename extract_type<Handles>::type & ... )> f,
			 Handles... h)
		:exprs(heap_copy(std::make_tuple(h...))),f(new std::function<T ()>([&,f,h...](){return f(h.get()...);})),
		 rs(new BitSet<HandleAbbrev>(setify(h.abbrev()...)))
		{}

	T operator()(const Store &) const {
		return (*f)();
	}

	BitSet<HandleAbbrev> getReadSet() const {
		return *rs;
	}
	
	template<typename F>
	FreeExpr(F f, Handles... h):FreeExpr(0, convert(f), h...){}
};

template<typename T, typename... H>
struct is_ConExpr<FreeExpr<T,H...> > : std::true_type {};

template<unsigned long long ID, typename T, typename... Vars>
auto find_usage(const FreeExpr<T,Vars...> &op){
	return fold(*op.exprs,
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

template<typename T, typename... Handles>
T run_expr(FreeExpr<T, Handles...> fe){
	return fe();
}


#define free_expr3(T,a,e) (FreeExpr<T,decltype(a)>([&](const typename extract_type<decltype(a)>::type &a){return e;},a))
#define free_expr4(T,a,b,e) (FreeExpr<T,decltype(a),decltype(b)>([&](const typename extract_type<decltype(a)>::type &a, \
																	 const typename extract_type<decltype(b)>::type &b){return e;},a,b))


#define free_expr_IMPL2(count, ...) free_expr ## count (__VA_ARGS__)
#define free_expr_IMPL(count, ...) free_expr_IMPL2(count, __VA_ARGS__)
#define free_expr(...) free_expr_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__)
