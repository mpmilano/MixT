#pragma once
#include "utils.hpp"
#include <type_traits>

namespace mutils{

	template<typename A, typename B>
	using TSeq = B;

	template<typename , typename, bool b>
	constexpr bool failOn(){
		static_assert(b,"Static assert failed");
		return true;
	}

#define sassert2(x,y,z) (mutils::failOn<x,y,z>())

#define tassert(x) decltype(sassert)

	template<typename T>
	using decay = typename std::decay<T>::type;

	template<template<typename> class Pred, typename Arg>
	using type_check = std::enable_if_t<Pred<Arg>::value, Arg>;

	template<template<typename> class Pred, typename Arg>
	struct neg_error_helper : std::integral_constant<bool, !Pred<Arg>::value >{
		static_assert(!Pred<Arg>::value,"Static assert error");
	};

	template<template<typename> class Pred, typename Arg>
	struct error_helper : std::integral_constant<bool, Pred<Arg>::value >{
		static_assert(Pred<Arg>::value,"Static assert error");
	};


	struct NoOverloadFoundError : MyriaException{
		const std::string mesg;
		NoOverloadFoundError(const decltype(mesg)& m):mesg(m){}
		const char * what() const _NOEXCEPT {
			return mesg.c_str();
		}
	};

#define DecayTraits(name) template<typename T> struct name<const T> : name<T> {}; \
	template<typename T> struct name<T&> : name<T> {};					\
	template<typename T> struct name<const T&> : name<T> {};

#define DecayTraitsLevel(name) template<Level l, typename T> struct name<l, const T> : name<l,T> {}; \
	template<Level l, typename T> struct name<l,T&> : name<l,T> {};		\
	template<Level l, typename T> struct name<l,const T&> : name<l,T> {};

	template<typename T> struct argument_type;
	template<typename T, typename U> struct argument_type<T(U)> { typedef U type; };
}
