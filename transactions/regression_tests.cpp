#include "flatten_expressions.hpp"
#include "mtl/insert_tracking.hpp"
#include "mtl/label_inference.hpp"
#include "mtl/split_phase.hpp"
#include "mtl/transaction.hpp"
#include "mtl/transaction_macros.hpp"
#include "parse_statements.hpp"
#include "testing_store/TestingStore.hpp"
#include "testing_store/mid.hpp"
#include "typecheck_and_label.hpp"
#include "typecheck_printer.hpp"
#include <iostream>

using namespace myria;
using namespace mtl;
using namespace parse_phase;
using namespace typecheck_phase;
using namespace testing_store;
using namespace label_inference;
using namespace tracking_phase;
using namespace split_phase;
using namespace tracker;

int main() {
  using BotStore = TestingStore<Label<bottom>>;
  using MidStore = TestingStore<Label<mid>>;
  using bot_int_handle = typename BotStore::template TestingHandle<int>;
  using mid_int_handle = typename MidStore::template TestingHandle<int>;
  BotStore store;
  MidStore midstore;
  constexpr int int_name = 23;
  bot_int_handle ih = store.template newObject<int>(nullptr, int_name, int_name);
  mid_int_handle mih = midstore.template newObject<int>(nullptr, int_name, int_name);
  using ClientTrk = ClientTracker<>;
  ClientTrk ct;
  //NOTE: should there be a flow from ih -> y = y?  YES argument: semantically deref of ih
  //happens before assignment and could fail.
  //No argument: this looks like a list of statements. It would be wrong to assume an ordering on them. 
  constexpr auto txn1 = TRANSACTION(remote y = mih, remote x = ih, y = y)::WITH(ih,mih);
  constexpr auto txn2 = TRANSACTION(remote ypre = mih, remote xpre = ih, var y = ypre, var x = xpre, y = y)::WITH(ih,mih);
  txn1.run_local(ct,ih,mih);
  txn2.run_local(ct,ih,mih);
  std::cout << txn1 << std::endl<< std::endl<< std::endl;
  std::cout << txn2 << std::endl<< std::endl<< std::endl;
}
