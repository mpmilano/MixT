#include "compile-time-tuple.hpp"

using namespace ct;

constexpr auto test = make_tuple(1,1);

int main(){
	return std::integral_constant<int,test.get<1>()>::value;
}
