//#define DECLARED_OPERATIONS template<typename T> DECLARE_OPERATION(Insert, RemoteObject<std::set<T> >*, const T& )
//#define STORE_LIST FileStore<Level::causal>,FileStore<Level::strong>


#include "IfBuilder.hpp"
#include "TransactionBuilder.hpp"
#include "Transaction.hpp"
#include "BaseCS.hpp"
#include "FileStore.hpp"
#include "Operate.hpp"
#include "TempBuilder.hpp"
#include "FreeExpr.hpp"
#include "FinalHeader.hpp"
#include "SQLStore.hpp"
//*/
#include "Transaction_macros.hpp"



namespace list_ex{
#include "list_example.cpp"
}

int main(){
	try {

		{

			//FileStore<Level::causal> fsc;
			auto &fs = FileStore<Level::strong>::filestore_instance();

			auto num_dir = fs.newCollection<HandleAccess::all, int>();
			{
				std::set<int> test;
				test.insert(13);
				num_dir.put(test);
				num_dir.get(); //just to see if it'll crash
			}

			TRANSACTION(
				let_mutable(tmp2) = true IN (
				let_mutable(tmp) = true IN (
					IF (tmp) THEN(
						do_op(Insert,num_dir,42)
						),
					tmp = free_expr(bool,num_dir, num_dir.empty());,
					WHILE (!tmp) DO (dummy1, tmp = true),
					IF (isValid(num_dir)) THEN (dummy1;))
					)
				);
		}

		std::cout << std::endl << "now doing causal" << std::endl << std::endl;

		{

			auto &fs = FileStore<Level::causal>::filestore_instance();

			auto num_dir = fs.newCollection<HandleAccess::all, int>();
			{
				std::set<int> test;
				test.insert(13);
				num_dir.put(test);
				num_dir.get(); //just to see if it'll crash
			}

			TRANSACTION(
					let_mutable(tmp) = true IN (
					IF (tmp) THEN(
						do_op(Insert,num_dir,42)
						),
					tmp = free_expr(bool,num_dir, num_dir.empty()),
					WHILE (!tmp) DO (dummy2, tmp = true),
					IF (isValid(num_dir)) THEN (dummy2;)
												 )
				);
		}

		//*/
	}
	catch (NoOverloadFoundError e){
		std::cerr << "No overload found: " << e.msg << std::endl;
	}

	SQLStore::SQLObject<int> i{
		SQLStore::GSQLObject{std::vector<char>()},
			std::unique_ptr<int>()};

	list_ex::main();

}
