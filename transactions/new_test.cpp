#include "IfBuilder.hpp"
#include "TransactionBuilder.hpp"
#include "Transaction.hpp"
#include "BaseCS.hpp"
#include "FileStore.hpp"
#include "Operate.hpp"
#include "Transaction_macros.hpp"

int main(){

	//FileStore<Level::causal> fsc;
	FileStore<Level::strong> fss;

	auto num_dir = fss.newCollection<HandleAccess::all, int>();
	{
		std::set<int> test;
		test.insert(13);
		num_dir.put(test);
		num_dir.get(); //just to see if it'll crash
	}

	/*
	TRANSACTION(
		let_mutable(tmp) = true IN (
			IF(tmp) THEN(
				do_op(Insert,num_dir,42)
				),
			tmp = false;,
			IF(!tmp) THEN (raw(dummy1))
			)
		);

	//*/
	{ TransactionBuilder<std::tuple<>, std::tuple<> > prev;
		{auto curr = [&]()
				{ auto tmp = (MutAssigner("tmp") = true );
				  ({auto curr = (make_if_begin((tmp)));
					  { auto prev2 = append(prev,curr); { auto prev = prev2;
							  {auto curr = (make_PreOp(Insert(constify(run_ast(mke_store(), op_arg(num_dir))),constify(run_ast(mke_store(), op_arg(42)))))(num_dir,42));
								  { auto prev2 = append(prev,curr); { auto prev = prev2;
										  {auto curr = (make_if_end()); { auto prev2 = append(prev,curr); { auto prev = prev2;
													  {auto curr = tmp = false;
														  { auto prev2 = append(prev,curr); { auto prev = prev2;
																  {auto curr = (make_if_begin((!tmp)));
																	  { auto prev2 = append(prev,curr); { auto prev = prev2;
																			  {auto curr = (dummy1); { auto prev2 = append(prev,curr); { auto prev = prev2;
																						  {auto curr = (make_if_end());
																							  { auto prev2 = append(prev,curr); { auto prev = prev2;
																									  return clobber(prev);}}}}}}}}}}}}}}}}}}}}});
				}();
			{ auto prev2 = append(prev,curr); { auto prev = prev2;
					Transaction ____transaction(prev); std::cout << ____transaction << std::endl; ____transaction();}}}}

}
