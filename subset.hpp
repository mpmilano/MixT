#include <tuple>
#include <type_traits>
#include <iostream>

//determines subsets of packs of POD.
template<typename FOO>
class subset{
private:
	template<typename... Args>
	static constexpr auto exists (Args...){
		static_assert(sizeof...(Args) == 0, "got template recursion wrong");
		return false;
	}

	template<typename T, typename... Args>
	static constexpr auto exists(T a, Args... b){
		return (sizeof...(Args) == 0 ? a 
				: (a ? true
				   : exists(b...)));
	}

	template<typename... Args>
	static constexpr auto forall (Args...){
		static_assert(sizeof...(Args) == 0, "got template recursion wrong");
		return true;
	}

	template<typename T, typename... Args>
	static constexpr auto forall(T a, Args... b){
		return (sizeof...(Args) == 0 ? a 
				: (a ? forall(b...) 
				   : false));
	}

public:
	template<FOO... FOOs>
	struct pack{};

private:
	template <typename C>
	static constexpr auto is_pack_f(C*)
		{return std::integral_constant<bool,false>();}

	template <FOO... Args>
	static constexpr auto is_pack_f(pack<Args...>*)
		{return std::integral_constant<bool,true>();}

	template <typename A>
	struct is_pack : decltype(is_pack_f((A*) nullptr)) {};
	
	template<FOO a>
	static constexpr bool equal(FOO b){
		return a == b;
	}

	template<FOO a, FOO... b>
	static constexpr bool is_in(pack<b...>){
		return exists(equal<a>(b)...);
	}

public:
	template<typename B, FOO... a>
	static constexpr bool f(pack<a...>, B){
		return forall(is_in<a>(B())... );
	}
};

template<bool b> auto mbool(){return b;}


int main(){
	return mbool<subset<int>::f(subset<int>::pack<1,2,3>(), subset<int>::pack<1,2>())>();
}
