#include "transaction.hpp"
#include "transaction_macros.hpp"
#include "split_printer.hpp"
#include <iostream>
using namespace myria;
using namespace mtl;
using namespace tracker;

int main(){
  Handle<myria::Label<myria::bottom>,bool> hndl;
  constexpr auto txn = TRANSACTION(remote _hndl = hndl, _hndl= hndl.isValid())::WITH(hndl);
  std::cout << txn << std::endl;
}
