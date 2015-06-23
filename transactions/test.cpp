#include "Transactions.hpp"

int main(){
	
	auto a = make_seq<Level::strong>(CSInt<Level::strong,0>());
	a,CSInt<Level::causal,1>();
}
