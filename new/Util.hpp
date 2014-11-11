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


#define catchall(name) template<typename.... Args> name (Args...)

#define cerror(test, message) static_assert(test, message); if (test) std::cerr << message << std::endl; ::exit(1)

#define carity(arity) cerror(sizeof...(Args) == (arity), "Error: function arity mismatch!")
