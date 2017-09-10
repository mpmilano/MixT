#include "parse_statements.hpp"
#include "flatten_expressions.hpp"
#include "typecheck_and_label.hpp"
#include <iostream>
#include "typecheck_printer.hpp"
#include "testing_store/TestingStore.hpp"
#include "mtl/label_inference.hpp"
#include "mtl/insert_tracking.hpp"
#include "mtl/split_phase.hpp"
#include "mtl/transaction.hpp"
#include "mtl/transaction_macros.hpp"
#include "testing_store/mid.hpp"
#include "mtl/RemoteList.hpp"

using namespace myria;
using namespace mtl;
using namespace parse_phase;
using namespace typecheck_phase;
using namespace testing_store;
using namespace label_inference;
using namespace tracking_phase;
using namespace split_phase;
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
	
	auto lst_ptr = ms.template newObject<IntList>(nullptr, 12, IntList{bs.template nullObject<int>(),ms.template nullObject<IntList>()});
	auto log_ptr = bs.template newObject<Log>(nullptr, 10, Log{});
	{
		using transaction_text = MUTILS_STRING({
		var lst = lst_ptr,
		var accum = 0,
		while (lst.isValid() || midtru){
			remote lst_value = lst->value,
			log_ptr.append(lst_value),
			lst = lst->next,
			accum = accum + lst_value,
			lst_value = lst_value + accum
		},
		return accum
			});
		    using parsed_t = DECT(parse_statement(transaction_text{}));
    {
      using namespace parse_phase;
      using flattened_t = DECT(parse_phase::flatten_expressions(parsed_t{}));
      {
        using namespace typecheck_phase;
        using checked_t = DECT(typecheck_phase::typecheck<1, 1>(
            typecheck_phase::type_environment<
                Label<top>,
						type_binding<MUTILS_STRING(midtru), bool, Label<mid>,type_location::local>,
			type_binding<MUTILS_STRING(lst_ptr), DECT(lst_ptr), Label<top>,type_location::local>,
			type_binding<MUTILS_STRING(log_ptr), DECT(log_ptr), Label<top>,type_location::local>
			>{},
            flattened_t{}));
        {
          using namespace label_inference;
					/*
  constexpr auto real_labels = collect_proper_labels(ast{});
  using constraints = DECT(minimize_constraints(collapse_constraints(collect_constraints(Label<top>{}, ast{}))));
					 */
					constexpr auto constraints = collapse_constraints(collect_constraints(Label<top>{}, checked_t{}));
					std::cout << constraints << std::endl;
					/*
          using inferred_t = DECT(infer_labels(checked_t{}));
          {
            using namespace tracking_phase;
            using tracked_t = DECT(insert_tracking_begin(inferred_t{}));
            using endorsed_one_t = DECT(do_pre_endorse(tracked_t{}));
            std::cout << endorsed_one_t{} << std::endl;
					}//*/
				}}}
	}
	/*
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

}
