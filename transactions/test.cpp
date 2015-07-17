#include "BaseCS.hpp"
#include "Seq.hpp"
#include "If.hpp"
#include "Assignmnet.hpp"
#include "Operate.hpp"
#include "Operation.hpp"
#include "Transaction.hpp"
#include "CommonExprs.hpp"
#include <iostream>

template<typename T>
constexpr T& id(const T& t){
	return t;
}

template<backend::Level l>
struct dummy_operation : public Operation<l, dummy_operation<l> >
{

	template<typename H, typename O>
	dummy_operation(const H&, const O&, BitSet<HandleAbbrev> bs):Operation<l, dummy_operation<l> >(bs){}

	void operator()(Store &){
		
	}
	
};

template<Level l>
void fooFight(const backend::DataStore::Handle<1,l, backend::HandleAccess::all, int>&){

}

make_operation(FooFight, fooFight);

int main(){

	using namespace backend;

	DataStore ds;
	backend::Client<1> interface(ds);

	auto thirteen = interface.newHandle<Level::strong>(13);

	//TODO: hybrids like the if-test.
	//Should be sufficient to extract test as strong -> casual
	//asignment to temp var, and use temp var for subsequent,
	//but need to make sure strong->causal readset dependency
	//is captured.  When to do the extraction?
	//can either defer both unpacking and extraction to
	//final pass during Transaction conversion, or can
	//try and add extraction to If-construction.
	
	BEGIN_TRANSACTION
		CSInt<Level::causal,1>() /
		IF (isValid(thirteen)) 
		THEN { CSInt<Level::causal,2>() /
			CSInt<Level::causal,3>() /
			CSInt<Level::causal,4>()
			}
		ELSE(causal) CSInt<Level::causal,3>()
		FI
		CSInt<Level::causal,0>() / 
	END_TRANSACTION;

	std::cout << "building transactions with macros and such" << std::endl;

	auto hndl = thirteen;
	
	auto a = make_seq(CSInt<Level::strong,0>());
	a / CSInt<Level::causal,1>();



	DummyConExpr<Level::strong> dummyExprStrong;
	DummyConExpr<Level::causal> dummyExprCausal;

	make_if(dummyExprStrong, a, a);

	std::cout << (CSInt<Level::causal,2>()) << std::endl<< std::endl;

	std::cout << make_if(dummyExprStrong, dummy2, dummy2).operator/
		(CSInt<Level::causal,2>()) << std::endl<< std::endl;
	
	std::cout << make_if(dummyExprStrong, dummy2, dummy2).operator/(
		CSInt<Level::strong,3>()) << std::endl<< std::endl;

	std::cout << make_if(dummyExprCausal, dummy2, dummy2).operator/(
		CSInt<Level::strong,3>()) << std::endl<< std::endl;

	hndl << 5 + 12;

	auto hndl2 = interface.newHandle<Level::causal>(17);

	int tmp = 14;

	//call an operation. 
	hndl.o<dummy_operation>(0);
	hndl.o<FooFight>(0);


	Store s;
	const auto &fe = free_expr2(bool, hndl, hndl2, return hndl + hndl2 + tmp;);
	bool b = fe(s);
	static_assert(get_level<decltype(fe)>::value == Level::causal,"");
	assert(b);

	auto fp = convert_fp([](int i, int j){return i + j;});
	fp(12,13);
	static_assert(std::is_same<decltype(fp),int (*) (int, int)>::value,"convert_fp lies!");


//*/
	std::cout << "all working" << std::endl;
}

