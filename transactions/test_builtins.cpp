#include <iostream>
#include "testing_store/TestingStore.hpp"
#include "mtl/transaction.hpp"
#include "mtl/transaction_macros.hpp"
#include "testing_store/mid.hpp"
#include "mtl/RemoteList.hpp"
#include "mtl/split_printer.hpp"

using namespace myria;
using namespace testing_store;
using namespace tracker;

int main(){
	//using TopStore = TestingStore<Label<top> >;
	//using MidStore = TestingStore<Label<mid> >;
	//using BotStore = TestingStore<Label<bottom> >;
	
	using ClientTrk = ClientTracker<>;
  ClientTrk ct;
	int three{3};
	constexpr auto txn = TRANSACTION(var lst = default list, lst.push_back(three), return lst)::WITH(three);
	std::cout << txn << std::endl;
}
