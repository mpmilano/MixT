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
	using MidStore = TestingStore<Label<mid> >;
	using BotStore = TestingStore<Label<bottom> >;
	
	using ClientTrk = ClientTracker<>;
  ClientTrk ct;

	using value_handle = typename BotStore::template TestingHandle<int>;

	using IntList = RemoteList<value_handle, MidStore::TestingHandle>;
	using Log = std::list<int>;

	MidStore ms;
	BotStore bs;
	
	auto lst_ptr = ms.template newObject<IntList>(nullptr, 12, IntList{bs.template newObject<int>(nullptr,13,13),ms.template nullObject<IntList>()});
	auto log_ptr = bs.template newObject<Log>(nullptr, 10, Log{});

	constexpr auto txn = TRANSACTION(
		var lst = lst_ptr,
		var accum = 0,
		while (lst.isValid()){
			remote lst_value = lst->value,
			log_ptr.append(lst_value),
			lst = lst->next,
			accum = accum + lst_value,
			lst_value = lst_value + accum
		},
		return accum
		)::WITH(lst_ptr,log_ptr);

		txn.run_local(ct,lst_ptr,log_ptr);//*/
		std::cout << txn << std::endl;

}
