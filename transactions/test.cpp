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
	typedef backend::DataStore::Handle<1,backend::Level::strong,backend::HandleAccess::all,int> H;
	
	dummy_operation(const H&){}

	//todo, maybe: it looks like build can't be templated.
	//this makes sense, because we're trying to convert it
	//to a stored function, which doesn't work if some of the template
	//arguments are left unfilled.
	
	static dummy_operation build(const H& r){
		return r;
	}
};


int main(){

	using namespace backend;

	auto a = make_seq(CSInt<Level::strong,0>());
	a,CSInt<Level::causal,1>();

	

	std::cout << (CSInt<Level::causal,2>()) << std::endl<< std::endl;

	std::cout << make_if(*((DummyConExpr<Level::strong>*) nullptr), dummy2, dummy2).operator,
		(CSInt<Level::causal,2>()) << std::endl<< std::endl;
	
	std::cout << make_if(*((DummyConExpr<Level::strong>*) nullptr), dummy2, dummy2).operator,(
		CSInt<Level::strong,3>()) << std::endl<< std::endl;

	std::cout << make_if(*((DummyConExpr<Level::causal>*) nullptr), dummy2, dummy2).operator,(
		CSInt<Level::strong,3>()) << std::endl<< std::endl;

	//wooo dereferencing null right off the bat!
	auto hndl = *(DataStore::Handle<1, Level::strong, HandleAccess::all, int>*) nullptr;

	hndl << 5 + 12;

	auto hndl2 = *(DataStore::Handle<1, Level::causal, HandleAccess::all, int>*) nullptr;

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
