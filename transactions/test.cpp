#include "BaseCS.hpp"
#include "Seq.hpp"
#include "If.hpp"
#include "Assignmnet.hpp"
#include <iostream>

int main(){

	using namespace backend;


	auto a = make_seq(CSInt<Level::strong,0>());
	a,CSInt<Level::causal,1>();

	std::cout << (CSInt<Level::causal,2>()) << std::endl<< std::endl;

	std::cout << make_if(*((ConExpr<Level::strong>*) nullptr), dummy2, dummy2).operator,
		(CSInt<Level::causal,2>()) << std::endl<< std::endl;
	
	std::cout << make_if(*((ConExpr<Level::strong>*) nullptr), dummy2, dummy2).operator,(
		CSInt<Level::strong,3>()) << std::endl<< std::endl;

	std::cout << make_if(*((ConExpr<Level::causal>*) nullptr), dummy2, dummy2).operator,(
		CSInt<Level::strong,3>()) << std::endl<< std::endl;

	//wooo dereferencing null right off the bat!
	auto hndl = *(DataStore::Handle<1, Level::strong, HandleAccess::all, int>*) nullptr;

	hndl << 5 + 12;
	
	auto hndl2 = *(DataStore::Handle<1, Level::causal, HandleAccess::all, int>*) nullptr;

	auto fe = free_expr2(bool, hndl, hndl2, return hndl + hndl2;);
	bool b = fe();
	static_assert(get_level<decltype(fe)>::value == Level::causal,"");
	assert(b);


//*/
	std::cout << "all working" << std::endl;
}
