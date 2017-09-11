#include "transaction.hpp"
#include "transaction_macros.hpp"
#include "split_printer.hpp"
#include "typecheck_printer.hpp"
#include "testing_store/TestingStore.hpp"
#include <iostream>
using namespace myria;
using namespace mtl;
using namespace tracker;

using test_struct = STRUCT(int, a, int, b, int, c);
namespace mutils{
		template<> struct typename_str<test_struct> {
			static std::string f(){return "test_struct";}
	};
}

int main(){
  
  test_struct tstruct2;
  test_struct tstruct;
  tstruct.a = 0;
  tstruct.b = 0;
  tstruct.c = 0;
  tstruct2.a = 0;
  tstruct2.b = 0;
  tstruct2.c = 0;
  using Store = testing_store::TestingStore<Label<bottom> >;
  Store store;
  auto hndl = store.template newObject<int>(nullptr, 12,12);
  auto hndl2 = store.template newObject<test_struct>(nullptr,5,test_struct{});

  /*
  {
	  using namespace myria;
	  using namespace mtl;
	  using namespace parse_phase;
	  using namespace typecheck_phase;
	  using namespace testing_store;
	  using namespace label_inference;
	  using namespace tracking_phase;
	  using namespace split_phase;
	  using namespace tracker;
  
  using transaction_text = MUTILS_STRING({var x = 3,
			  var y = 5,
			  x = 7,
			  y = y + x,
			  var condition = 0,
			  while (condition < x) {
			    y = y + 3,
			    condition = tstruct2.a + 1,
			    tstruct2.a = condition,
			    remote z = hndl,
			    z = condition,
			    var unused = *hndl,
			    unused = hndl2->a - 1,
			    z = unused,
			    var a = z,
			    z = a
			  },
			  if (45 > y) {
			    y = tstruct.a,
			    tstruct.b = x
			  } else {if (hndl.isValid()){return 7} else {}}});
      using parsed_t = DECT(parse_statement(transaction_text{}));
    {
      using namespace parse_phase;
      using flattened_t = DECT(parse_phase::flatten_expressions(parsed_t{}));
      {
        using namespace typecheck_phase;
        using checked_t = DECT(typecheck_phase::typecheck<1, 1>(
            typecheck_phase::type_environment<
                Label<top>,
			type_binding<MUTILS_STRING(tstruct), DECT(tstruct), Label<top>,type_location::local>,
			type_binding<MUTILS_STRING(tstruct2), DECT(tstruct2), Label<top>,type_location::local>,
			type_binding<MUTILS_STRING(hndl), DECT(hndl), Label<top>,type_location::local>,
			type_binding<MUTILS_STRING(hndl2), DECT(hndl2), Label<top>,type_location::local>
			>{},
            flattened_t{}));
        {
          using namespace label_inference;
          using inferred_t = DECT(infer_labels(checked_t{}));
          {
            using namespace tracking_phase;
            using tracked_t = DECT(insert_tracking_begin(inferred_t{}));
            using endorsed_one_t = DECT(do_pre_endorse(tracked_t{}));
            std::cout << tracked_t{} << std::endl;
            std::cout << endorsed_one_t{} << std::endl;

            using split_t = DECT(split_computation<
                                 endorsed_one_t,
								 type_binding<MUTILS_STRING(tstruct), DECT(tstruct), Label<top>,type_location::local>,
								 type_binding<MUTILS_STRING(tstruct2), DECT(tstruct2), Label<top>,type_location::local>,
								 type_binding<MUTILS_STRING(hndl), DECT(hndl), Label<top>,type_location::local>,
								 type_binding<MUTILS_STRING(hndl2), DECT(hndl2), Label<top>,type_location::local>
                                 >());
            using recollapsed_t = DECT(recollapse(split_t{}));
            //std::cout << recollapsed_t{} << std::endl;
			std::cout << runnable_transaction::relabel(recollapsed_t{}) << std::endl;
          }
        }
      }
    }
}//*/
  

  constexpr auto txn = TRANSACTION(var x = 3,
			  var y = 5,
			  x = 7,
			  y = y + x,
			  var condition = 0,
			  while (condition < x) {
			    y = y + 3,
			    condition = tstruct2.a + 1,
			    tstruct2.a = condition,
			    remote z = hndl,
			    z = condition,
			    var unused = *hndl,
			    unused = hndl2->a - 1,
			    z = unused,
			    var a = z,
			    z = a
			  },
			  if (45 > y) {
			    y = tstruct.a,
			    tstruct.b = x
			  } else {if (hndl.isValid()){return 7} else {}}
				   )::WITH(tstruct,tstruct2,hndl,hndl2);//*/
  //txn.just_print_it();
	/*
  constexpr 
#include "test_complex_transaction_precompiled.incl"
    txn{};    //*/
std::cout << txn << std::endl;
	

	ClientTracker<> trk;
	txn.run_local(trk,tstruct,tstruct2,hndl,hndl2);//*/
}
