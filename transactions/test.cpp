#include "BaseCS.hpp"
#include "Seq.hpp"
#include "If.hpp"
#include "Assignmnet.hpp"
#include "Operate.hpp"
#include "Operation.hpp"
#include "Transaction.hpp"
#include "CommonExprs.hpp"
#include "TypeMap.hpp"
#include "FileStore.hpp"
#include "FreeExpr.hpp"
#include <iostream>

template<typename T>
constexpr T& id(const T& t){
	return t;
}


struct test_entry{
	unsigned long long key = 12;
	typedef int v1;
	static constexpr int v2 = 0;
}; 

int main(){

	//TODO: hybrids like the if-test.
	//Should be sufficient to extract test as strong -> casual
	//asignment to temp var, and use temp var for subsequent,
	//but need to make sure strong->causal readset dependency
	//is captured.  When to do the extraction?
	//can either defer both unpacking and extraction to
	//final pass during Transaction conversion, or can
	//try and add extraction to If-construction.

	FileStore<Level::causal> fsc;
	FileStore<Level::strong> fss;

	auto num_dir = fss.newCollection<HandleAccess::all, int>();

	std::set<int> test;
	test.insert(13);
	num_dir.put(test);

	num_dir.get();

	auto thirteen = fss.newObject<HandleAccess::all,int>();
	thirteen.put(13);
	auto five = fsc.newObject<HandleAccess::all,int>();
	five.put(5);

	auto ro_thirteen = thirteen.readOnly();


	BEGIN_TRANSACTION
		(temp(Level::causal,int,"f") = 6)/
		do_op(Insert,num_dir,42) /
		do_op(Insert,num_dir,(CSInt<Level::strong,42>())) /
		IF (isValid(thirteen)) 
		THEN { CSInt<Level::causal,2>() /
			CSInt<Level::causal,3>() /
			CSInt<Level::causal,4>() /
			ref("f")
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

	auto hndl2 = fsc.newObject<HandleAccess::all, int>();

	int tmp = 14;


	Store s;
	const auto &fe = free_expr(bool, hndl, hndl2, return hndl + hndl2 + tmp;);
	bool b = fe(s);
	static_assert(get_level<decltype(fe)>::value == Level::causal,"");
	assert(b);

	auto fp = convert_fp([](int i, int j){return i + j;});
	fp(12,13);
	static_assert(std::is_same<decltype(fp),int (*) (int, int)>::value,"convert_fp lies!");

	typedef ctm::insert<12,
						int,
						typename std::integral_constant<int, 6>::type,
						ctm::empty_map> map2;

	typedef ctm::find<12,map2> found;
	std::cout << type_name<found>() << std::endl;
	
	static_assert(found::v2::value == 6);

	static_assert(found::found::value);
	static_assert(!ctm::find<13,map2>::found::value);


//*/
	std::cout << "all working" << std::endl;
}

