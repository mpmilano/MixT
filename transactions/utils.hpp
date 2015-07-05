#pragma once
#include "restrict.hpp"
#include "args-finder.hpp"
#include <type_traits>
#include <cassert>
#include <tuple>
#include <set>

template<typename T, std::size_t size1, std::size_t size2>
auto prefix_array(const std::array<T,size1>& t,
				  const std::array<T,size2> &arr,
				  const T& remove){
	assert(remove == t[0]);
	assert(remove == arr[0]);
	std::array<T, size1 + size2 - 1 > ret;
	std::size_t i = 0;
	for (; i < size1; ++i){
		ret[i] = t[i];
	}
	std::size_t j = 1;
	for (; i < size1 + size2 -1 ; ++i){
		ret[i] = arr[j];
		++j;
	}
	return ret;
}

template<typename T>
constexpr auto mke(){
	return *((T*) nullptr);
}

template<typename T>
constexpr auto mke_p(){
	return ((T*) nullptr);
}


template<typename T>
T cpy(const T& t){
	return T(t);
}

template<int ...>
struct seq { };

template<int N, int ...S>
struct gens : gens<N-1, N-1, S...> { };

template<int ...S>
struct gens<0, S...> {
	typedef seq<S...> type;
	static type build(){ return type();}
};


template<typename F, typename Tuple, int ...S>
auto __callFunc(const F& f, const Tuple &t, seq<S...>) {
	return f(std::get<S>(t)...);
}


template<typename F, typename Tuple, typename Pack, restrict(!std::is_function<F>::value)>
auto callFunc(const F &f, const Tuple &t, Pack p) {
	return __callFunc(f,t,p);
}

template<typename Ret, typename Tuple, typename Pack, typename... Args>
Ret callFunc(Ret (*f) (Args...), const Tuple &t, Pack p) {
	return __callFunc(convert(f),t,p);
}

template<typename Ret, typename Tuple, int ...S>
Ret callConstructor(const Tuple &t, seq<S...>) {
	return Ret(std::get<S>(t)...);
}



/*
template<typename Ret, typename Tuple, int ...S>
Ret callFunc_cr(Ret (*f) (typename std::add_const<
					     typename std::add_lvalue_reference<
					       typename std::decay<
					         decltype(std::get<S>(mke<Tuple>()))
					       >::type
					     >::type
					   >::type...),
			 const Tuple &t, seq<S...>) {
	return f(std::get<S>(t)...);
}
*/
