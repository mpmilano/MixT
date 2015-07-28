#pragma once
#include <type_traits>

template<typename A, typename B>
using TSeq = B;

template<typename , typename, bool b>
constexpr bool failOn(){
	static_assert(b);
	return true;
}

#define sassert2(x,y,z) (failOn<x,y,z>())

#define tassert(x) decltype(sassert)

template<typename T>
using decay = typename std::decay<T>::type;
