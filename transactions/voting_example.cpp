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

struct election {

	using candidate = int;
	
	using MidStore = TestingStore<Label<mid> >;
	using BotStore = TestingStore<Label<bottom> >;
	using candidate_handle = typename MidStore::template TestingHandle<candidate>;

	candidate_handle candidate1;
	candidate_handle candidate2;
	
	using ClientTrk = ClientTracker<>;
  ClientTrk ct;

	election(DECT(candidate1) c1, DECT(candidate2) c2)
		:candidate1(c1), candidate2(c2){}

	void vote(candidate_handle candidate){
		return TRANSACTION(
			candidate.increment()
			).RUN_LOCAL_WITH(ct,candidate);
	}

	auto tally(){
		 TRANSACTION(
			var lst = default list,
			lst.push_back(candidate1.consistent_read().endorse(mid)),
			lst.push_back(candidate2.consistent_read().endorse(mid)),
			return lst).RUN_LOCAL_WITH(ct,candidate1,candidate2);
	}
	
};
	
int main(){

}
