#pragma once
#include <type_traits>

template<typename... Args>
constexpr auto exists (Args...){
	static_assert(sizeof...(Args) == 0, "got template recursion wrong");
	return false;
}

template<typename T, typename... Args>
constexpr auto exists(T a, Args... b){
	return (sizeof...(Args) == 0 ? a 
		: (a ? a 
		   : exists(b...)));
}


template<typename... Args>
constexpr auto forall (Args...){
	static_assert(sizeof...(Args) == 0, "got template recursion wrong");
	return true;
}

template<typename T, typename... Args>
constexpr auto forall(T a, Args... b){
	return (sizeof...(Args) == 0 ? a 
		: (a ? forall(b...) 
		   : false));
}


int gensym(){
	static int counter = 0;
	return counter++;
}


