#pragma once
#include "restrict.hpp"
#include <type_traits>
#include <cassert>
#include <tuple>
#include "utils.hpp"

namespace aux{
	template<std::size_t...> struct seq{};

	template<std::size_t N, std::size_t... Is>
	struct gen_seq : gen_seq<N-1, N-1, Is...>{};

	template<std::size_t... Is>
	struct gen_seq<0, Is...> : seq<Is...>{};

	template<class Ch, class Tr, class Tuple, std::size_t... Is>
	void print_tuple(std::basic_ostream<Ch,Tr>& os, Tuple const& t, seq<Is...>){
		using swallow = int[];
		(void)swallow{0, (void(os << (Is == 0? "" : ", ") << std::get<Is>(t)), 0)...};
	}
} // aux::

template<class Ch, class Tr, class... Args>
auto operator<<(std::basic_ostream<Ch, Tr>& os, std::tuple<Args...> const& t)
	-> std::basic_ostream<Ch, Tr>&
{
	os << "(";
	aux::print_tuple(os, t, aux::gen_seq<sizeof...(Args)>());
	return os << ")";
}


template<typename... T>
constexpr bool is_tuple_f(const std::tuple<T...>*){
	return true;
}

template<typename T>
constexpr bool is_tuple_f(const T*){
	return false;
}

template<typename T>
struct is_tuple : std::integral_constant<bool, is_tuple_f((T*) nullptr)>::type {};
//*/

template<int index, int max, typename Acc, typename F, typename Tuple1, typename Tuple2,
		 restrict(index < max)>
auto tuple_2fold_impl(const F& fun, const Tuple1 &t1, const Tuple2 &t2, const Acc &acc, const std::true_type::type* ){
	auto acc2 = fun(std::get<index>(t1),std::get<index>(t2),acc);
	return tuple_2fold_impl<index+1, max, decltype(acc2), F, Tuple1, Tuple2>
		(fun,t1,t2,acc2, (typename std::integral_constant<bool,(index + 1) < max>::type*) nullptr);
}
	
template<int index, int max, typename Acc, typename F, typename Tuple1, typename Tuple2,
			 restrict(index == max)>
auto tuple_2fold_impl(const F& , const Tuple1 &, const Tuple2 &, const Acc &acc, const std::false_type::type*){
	return acc;
}


template<typename Tuple1, typename Tuple2, typename F, typename Acc>
auto tuple_2fold(const F& f, const Tuple1 &t1, const Tuple2 &t2, const Acc &acc){
	static_assert(is_tuple<Tuple1>::value && is_tuple<Tuple2>::value, "These are not tuples.  Try again");
	static_assert(std::tuple_size<Tuple1>::value == std::tuple_size<Tuple2>::value,"Please only double-fold equal-length things");

	constexpr int size1 = std::tuple_size<Tuple1>::value;
	constexpr int size2 = std::tuple_size<Tuple2>::value;
	assert(size1 == size2);
	
	return tuple_2fold_impl<0,size1,Acc,F,Tuple1,Tuple2>
		(f,t1,t2,acc,
		 (typename std::integral_constant<bool,(size1 > 0)>::type*) nullptr);
}

template<int ind, int stop, typename Acc, typename F, typename... Args, restrict(ind < stop)>
auto constexpr fold_recr(const std::tuple<Args...> &vec, const F &f, const Acc &acc){
	return fold_recr<ind+1,stop>(vec,f,f(std::get<ind>(vec),acc));
}
	
template<int ind, int stop, typename Acc, typename F, typename... Args>
typename std::enable_if<ind == stop,Acc>::type
constexpr fold_recr(const std::tuple<Args...> &, const F &, const Acc &acc){
	return acc;
}

template<typename Acc, typename F, typename... Args>
auto constexpr fold_(const std::tuple<Args...> &vec, const F &f, const Acc & acc){
	return fold_recr<0,sizeof...(Args)>(vec,f,acc);
}

template<typename Acc, typename F, typename Tuple, restrict(!std::is_function<F>::value)>
auto constexpr fold(const Tuple &vec, const F &f, const Acc & acc){
	return fold_(vec,f,acc);
}

template<typename Acc, typename Ret, typename Tuple, typename... Args>
auto constexpr fold(const Tuple &vec, Ret (*f)(Args...), const Acc & acc){
	return fold_(vec,convert(f),acc);
}

template<template<typename> class Pred, typename... Args>
constexpr bool forall_types_f(const std::tuple<Args...>*){
	return forall(Pred<Args>::value...);
}

template<template<typename> class Pred, typename T>
struct forall_types : std::integral_constant<bool,forall_types_f<Pred>(mke_p<T>())> {};


template<typename tuple, int... S>
constexpr bool _forall(const tuple &t, const seq<S...>&){
	return forall(std::get<S>(t)...);
}


template<typename... Args>
constexpr bool forall(const std::tuple<Args...> &t){
	return _forall(t,gens<sizeof...(Args)>::build());
}

template<typename tuple, int... S>
constexpr bool _exists(const tuple &t, const seq<S...>&){
	return exists(std::get<S>(t)...);
}


template<typename... Args>
constexpr bool exists(const std::tuple<Args...> &t){
	return _exists(t,gens<sizeof...(Args)>::build());
}


template<template<typename, typename> class Func, typename Accum, typename Arg>
struct fold_types_str;

template<template<typename, typename> class Func, typename Accum>
struct fold_types_str<Func,Accum,std::tuple<> >{
	typedef Accum type;
};

template<template<typename, typename> class Func, typename Accum, typename Arg1, typename... Args>
struct fold_types_str<Func,Accum,std::tuple<Arg1, Args...> > :
	fold_types_str<Func,Func<Arg1,Accum>, std::tuple<Args...> > {};

template<template<typename, typename> class Func, typename T, typename Accum>
using fold_types = typename fold_types_str<Func,Accum,T>::type;


template <typename, typename> struct Cons;

template <typename  T, typename... Args>
struct Cons<T, std::tuple<Args...> >{
	typedef std::tuple<T, Args...> type;
};

template<typename> struct _Rest;

template<typename A, typename... Contents>
struct _Rest<std::tuple<A, Contents...> >{
	typedef std::tuple<Contents...> type;
};

template<typename T>
using Rest = typename _Rest<T>::type;

template<typename> struct _First;

template<typename A, typename... Contents>
struct _First<std::tuple<A, Contents...> >{
	typedef A type;
};

template<typename T>
using First = typename _First<T>::type;

template<typename T, typename... Args>
typename Cons<T,std::tuple<Args...> >::type
tuple_cons(const T &t, const std::tuple<Args...> &a){
	return std::tuple_cat(std::make_tuple(t),a);
}

template<template<typename> class, typename>
struct extract_match_str;

template<template<typename> class Pred, typename Fst, typename... Rst>
struct extract_match_str<Pred,std::tuple<Fst,Rst...> >{
	using type = typename std::conditional<
		Pred<Fst>::value,
		Fst,
		typename extract_match_str<Pred,std::tuple<Rst...> >::type>::type;
};

template<template<typename> class Pred, typename Fst>
struct extract_match_str<Pred,std::tuple<Fst> >{
	using type = Fst;
};

template<template<typename> class Pred, typename... T>
using extract_match = typename extract_match_str<Pred,std::tuple<T...> >::type;
