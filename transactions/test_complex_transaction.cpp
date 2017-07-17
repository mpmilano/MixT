#include "transaction.hpp"
#include "transaction_macros.hpp"
#include "split_printer.hpp"
#include <iostream>
using namespace myria;
using namespace mtl;
using namespace tracker;

int main(){
  using test_struct = STRUCT(int, a, int, b, int, c);
  test_struct tstruct2;
  test_struct tstruct;
  tstruct.a = 0;
  tstruct.b = 0;
  tstruct.c = 0;
  tstruct2.a = 0;
  tstruct2.b = 0;
  tstruct2.c = 0;
  Handle<myria::Label<myria::bottom>,int> hndl;
  Handle<myria::Label<myria::bottom>,test_struct> hndl2;

  /*
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

  constexpr 
#include "test_complex_transaction_precompiled.incl"
    txn{};    //*/

  std::cout << txn << std::endl;
	

	ClientTracker<> trk;
		txn.run_local(trk,tstruct,tstruct2,hndl,hndl2);//*/
}
