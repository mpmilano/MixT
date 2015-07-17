#pragma once
#include "restrict.hpp"
#include "args-finder.hpp"
#include <type_traits>
#include <cassert>
#include <tuple>
#include <set>
#include <map>

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
constexpr T mke(){
	return *((typename std::decay<T>::type*) nullptr);
}

template<typename T>
constexpr auto mke_p(){
	return ((typename std::decay<T>::type*) nullptr);
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

template<typename Arg>
constexpr Arg last_of_f(const std::tuple<Arg>*){
	return mke<Arg>();
}

template<typename Arg1, typename... Args>
constexpr decltype(last_of_f(mke_p<std::tuple<Args...> >()))
	last_of_f(const std::tuple<Arg1, Args...>*){
	return last_of_f(mke_p<std::tuple<Args...> >());
}

template<typename... T>
struct last_of {
	static_assert(sizeof...(T) > 0, "Error: cannot call last_of on empty packs");
	using type = decltype(last_of_f(mke_p<std::tuple<typename std::decay<T>::type...> >()));
};

//TODO: define this better and move it.
struct Store : std::map<int,std::unique_ptr<void*> >{
	bool contains(int i) const{
		return this->find(i) != this->end();
	}

	typedef void** stored;

	Store(){}

	Store(const Store&) = delete;
};
