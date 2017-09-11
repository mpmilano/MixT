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
#include "mtl/relabel.hpp"
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
  int seven = 7;
  using ClientTrk = ClientTracker<>;
  ClientTrk ct;
  {
	  using transaction_text = MUTILS_STRING({remote y = mih, remote x = ih, y = (x * 2).endorse(mid), return y});
    using parsed_t = DECT(parse_statement(transaction_text{}));
    {
      using namespace parse_phase;
      using flattened_t = DECT(parse_phase::flatten_expressions(parsed_t{}));
      {
        using namespace typecheck_phase;
        using checked_t = DECT(typecheck_phase::typecheck<1, 1>(
            typecheck_phase::type_environment<
                Label<top>,
			type_binding<MUTILS_STRING(ih), DECT(ih), Label<top>,type_location::local>,
			type_binding<MUTILS_STRING(mih), DECT(mih), Label<top>,type_location::local>
			>{},
            flattened_t{}));
        {
					std::cout << checked_t{} << std::endl;
          using namespace label_inference;
          using inferred_t = DECT(infer_labels(checked_t{}));
          {
            using namespace tracking_phase;
            using tracked_t = DECT(insert_tracking_begin(inferred_t{}));
            std::cout << tracked_t{} << std::endl;


            using endorsed_one_t = DECT(do_pre_endorse(tracked_t{}));
						std::cout << endorsed_one_t{} << std::endl;
						/*
            using split_t = DECT(split_computation<
                                 endorsed_one_t,
                                 type_binding<MUTILS_STRING(ih), DECT(ih),
                                              Label<top>,
								 type_location::local>,
								 type_binding<MUTILS_STRING(mih), DECT(mih), Label<top>,type_location::local>>());
            using recollapsed_t = DECT(recollapse(split_t{}));
            //std::cout << recollapsed_t{} << std::endl;
			std::cout << runnable_transaction::relabel(recollapsed_t{}) << std::endl;//*/
          }
        }
      }
    }
  }
	(void) seven;
	(void) ct;
	

	TRANSACTION(remote y = mih, remote x = ih, y = (x * 2).endorse(mid), return y)::WITH(mih,ih).run_local(ct,mih,ih);
  auto fourteen =
      TRANSACTION(return (seven * 2).endorse(mid))::WITH(seven).run_local(
          ct, seven);
  auto fourty_six = TRANSACTION(remote x = ih, return (x * 2).endorse(mid))::WITH(ih).run_local(ct, ih);
  assert(fourteen == seven * 2);
  assert(fourty_six == 46);
  std::cout << fourty_six << std::endl;//*/
}
