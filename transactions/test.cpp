#include "BaseCS.hpp"
#include "Seq.hpp"
#include "If.hpp"
#include "Assignmnet.hpp"
#include "Operate.hpp"
#include "Operation.hpp"
#include <iostream>

template<typename T>
constexpr T& id(const T& t){
	return t;
}

struct dummy_operation : public Operation<backend::Level::strong, backend::HandleAccess::all, dummy_operation>
{

	template<typename H>
	dummy_operation(const H&){}
	
};


int main(){

	using namespace backend;

	auto a = make_seq(CSInt<Level::strong,0>());
	a,CSInt<Level::causal,1>();

	DataStore ds;
	backend::Client<1> interface(ds);


	DummyConExpr<Level::strong> dummyExprStrong;
	DummyConExpr<Level::causal> dummyExprCausal;

	make_if(dummyExprStrong, a, a);

	std::cout << (CSInt<Level::causal,2>()) << std::endl<< std::endl;

	std::cout << make_if(dummyExprStrong, dummy2, dummy2).operator,
		(CSInt<Level::causal,2>()) << std::endl<< std::endl;
	
	std::cout << make_if(dummyExprStrong, dummy2, dummy2).operator,(
		CSInt<Level::strong,3>()) << std::endl<< std::endl;

	std::cout << make_if(dummyExprCausal, dummy2, dummy2).operator,(
		CSInt<Level::strong,3>()) << std::endl<< std::endl;

	//wooo dereferencing null right off the bat!
	auto hndl = interface.newHandle<Level::strong>(13);

	hndl << 5 + 12;

	auto hndl2 = interface.newHandle<Level::causal>(17);

	int tmp = 14;

	//call an operation. 
	hndl.o<dummy_operation>(0);
	
	auto fe = free_expr2(bool, hndl, hndl2, return hndl + hndl2 + tmp;);
	bool b = fe();
	static_assert(get_level<decltype(fe)>::value == Level::causal,"");
	assert(b);


//*/
	std::cout << "all working" << std::endl;
}
