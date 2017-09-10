#include "transaction.hpp"
#include "transaction_macros.hpp"
#include "split_printer.hpp"
#include <iostream>
using namespace myria;
using namespace mtl;
using namespace tracker;

constexpr auto mk_txn(const Handle<myria::Label<myria::bottom>,bool> &hndl){
	return TRANSACTION(remote _hndl = hndl, _hndl= hndl.isValid())::WITH(hndl);
}

int main(){
	using txn_t = DECT(mk_txn(std::declval<Handle<myria::Label<myria::bottom>,bool> >()));
  txn_t txn{};
  std::cout << txn << std::endl;
}
