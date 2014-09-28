#pragma once

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

template<typename T>
constexpr bool bool_cast(T t){
	return t;
}

template<typename Expr, typename... T>
struct forall : std::integral_constant <bool, forall(bool_cast(Expr<T>)...)> {};


template<typename T, typename Maybe>
class is_T_helper{
	static constexpr bool test(T* t){return true;}
	template<typename _t>
	static constexpr bool test(_t ){return false;}

	static constexpr bool value = test( (Maybe*) nullptr);

};

template<typename T, typename Maybe>
struct is_T : public std::integral_constant <bool,is_T_helper<T,Maybe>::value > {};

