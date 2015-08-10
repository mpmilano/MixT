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

template<typename T, typename V = void>
using enable_if = typename std::enable_if<T,V>::type;

template<template<typename> typename Pred, typename Arg>
using type_check = enable_if<Pred<Arg>::value, Arg>;
