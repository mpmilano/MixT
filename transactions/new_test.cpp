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

	TRANSACTION(
		let_mutable(tmp) = true IN (
			IF(tmp) THEN(
				raw(dummy1) //do_op(Insert,num_dir,42);
				)
			)
		);
}
