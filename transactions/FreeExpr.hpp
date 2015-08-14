#pragma once
#include "utils.hpp"
#include "tuple_extras.hpp"
#include "type_utils.hpp"
#include "ConExpr.hpp"

template<Level l, typename T>
struct getsT {
	virtual T get(const Store &) const = 0;
	static constexpr Level level = l;
};

template<Level, typename T, typename H>
struct putT;

template<typename T, typename H, HandleAccess ha, Level l>
struct putT<l,T,Handle<l,ha,H> > : public getsT<l,T> {
	const Handle<l,ha,H> h;

	putT(const Handle<l,ha,H>&h):h(h){}
	
	T get(const Store &) const {
		return h.get();
	};
};

template<Level l, typename T>
struct putT<l,T,T > : public getsT<l,T> {
	const T t;

	putT(const T&t):t(t){}
	
	T get(const Store &) const {
		return t;
	};
};


template<typename T>
auto make_gets(const T &t){
	auto* p = new putT<get_level<T>::value, typename extract_type<T>::type, T>{t};
	return p;
}

template<typename T>
using gets_transform = const std::shared_ptr<const getsT<get_level<T>::value, typename extract_type<T>::type > >;

template<typename T, typename... Exprs>
struct FreeExpr : public ConExpr<T, min_level<Exprs...>::value > {

	//this one is just for temp-var-finding
	const std::tuple<Exprs...> params;
	
	const std::tuple<gets_transform<Exprs>...> exprs;
	const std::shared_ptr<const std::function<T (const Store&, const std::tuple<gets_transform<Exprs> ...>& )> > f;
	const std::shared_ptr<const BitSet<HandleAbbrev> > rs;
	const int id = gensym();
	
	FreeExpr(int,
			 std::function<T (const typename extract_type<Exprs>::type & ... )> f,
			 Exprs... h)
		:params(std::make_tuple(h...)),
		 exprs(std::make_tuple(make_gets(h)...)),
		 f(heap_copy(convert([=](const Store &s, const std::tuple<gets_transform<Exprs> ...> &t){
					 auto retrieved = fold(t,
										   [&s](const auto &e, const auto &acc){return tuple_cons(e->get(s),acc);}
										   ,std::tuple<>());
					 return callFunc(f,retrieved);
					 }))),
		 rs(new BitSet<HandleAbbrev>(setify(h.abbrev()...)))
		{}

	T operator()(Store &s) const {
		{
			decltype(exprs) new_exprs =
				fold(exprs,
					 [&s](const auto &e, const auto &acc){
						 if (e->level == Level::strong){
							 typedef decltype(e->get(s)) T2;
							 constexpr Level l = decay<decltype(*e)>::level;
							 typedef const std::shared_ptr<const getsT<l,T2> > nt;
							 nt n(heap_copy(putT<l,T2,T2>{e->get(s)}));
							 return tuple_cons(n ,acc);
						 }
						 else return tuple_cons(e,acc);
					 },
					 std::tuple<>()
					);
			s.insert(id,new_exprs);
		}

		//some time later

		{
			const decltype(exprs) &new_exprs = s.get<decltype(exprs)>(id);
			return (*f)(s,new_exprs);
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
