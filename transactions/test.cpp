#include "BaseCS.hpp"
#include "Seq.hpp"
#include "If.hpp"

int main(){
	
	auto a = make_seq(CSInt<Level::strong,0>());
	a,CSInt<Level::causal,1>();
	assert(get_level<decltype(a)>::value == Level::strong);
}
