#include "BaseCS.hpp"
#include "Seq.hpp"
#include "If.hpp"
#include <iostream>

int main(){
	
	auto a = make_seq(CSInt<Level::strong,0>());
	a,CSInt<Level::causal,1>();

	std::cout << (CSInt<Level::causal,2>()) << std::endl<< std::endl;

	std::cout << make_if(*((ConExpr<Level::strong>*) nullptr), dummy2, dummy2).operator,
		(CSInt<Level::causal,2>()) << std::endl<< std::endl;
	
	std::cout << make_if(*((ConExpr<Level::strong>*) nullptr), dummy2, dummy2).operator,(
		CSInt<Level::strong,3>()) << std::endl<< std::endl;

	std::cout << make_if(*((ConExpr<Level::causal>*) nullptr), dummy2, dummy2).operator,(
		CSInt<Level::strong,3>()) << std::endl<< std::endl; 
	
//*/
	std::cout << "all working" << std::endl;
}
