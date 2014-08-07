#include <tuple>
#include <type_traits>
#include <iostream>
#include "extras"

//determines subsets of packs of POD.
template<typename FOO>
class subset{
public:
	template<FOO... FOOs>
	struct pack{};

private:

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
