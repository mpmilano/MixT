#include "mtl/transaction.hpp"
#include "mtl/transaction_macros.hpp"

using namespace myria;
using namespace mtl;

int main(){
  using Hndl = Handle<myria::Label<myria::bottom>,int>;
  using test_struct = STRUCT(Hndl, hndl);
  Handle<myria::Label<myria::bottom>,test_struct> tstruct;
  constexpr auto txn = TRANSACTION(var x = tstruct->hndl,
				   var y = *x,
				   if (false) {return y}
				   else {return (*tstruct->hndl) - 1})
    .WITH(tstruct);
  std::cout << txn << std::endl;
}
