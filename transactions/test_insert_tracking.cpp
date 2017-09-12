#include "pgsql/SQLStore.hpp"
#include "mtl/transaction.hpp"
#include "mtl/transaction_macros.hpp"
#include "FinalHeader.hpp"
#include "mutils/CTString.hpp"
#include "mutils/CTString_macro.hpp"
#include "mtl/common_strings.hpp"
#include "mtl/AST_parse.hpp"
#include "mtl/AST_typecheck.hpp"
#include "mtl/parse_statements.hpp"
#include "mtl/parse_expressions.hpp"
#include "mtl/typecheck_and_label.hpp"
#include "myria-utils/utils.hpp"
#include "mtl/flatten_expressions.hpp"
#include "mtl/split_phase.hpp"
#include "mtl/typecheck_printer.hpp"
#include "mtl/label_inference.hpp"
#include "mtl/parse_printer.hpp"
#include "mtl/split_printer.hpp"
#include "mtl/interp.hpp"
#include "mtl/transaction_macros.hpp"

using namespace myria;
using namespace server;
using namespace pgsql;
using namespace mtl;
using namespace tracker;
using namespace std;
using namespace mutils;
using namespace typecheck_phase;
using namespace label_inference;
using Hndl1 = Handle<Label<pgsql::causal >, int, SupportedOperation<RegisteredOperations::Increment,void,SelfType> >;
using Hndl2 = Handle<Label<pgsql::strong >, int, SupportedOperation<RegisteredOperations::Increment,void,SelfType> >;

int main(){
	SQLConnectionPool<Level::strong> sp;
	SQLConnectionPool<Level::causal> cp;
	typename SQLStore<Level::causal>::SQLInstanceManager ci{cp};
	typename SQLStore<Level::strong>::SQLInstanceManager si{sp};
	DeserializationManager dsm{{&si,&ci}};
  Hndl1 hndl1 = ci.inst().template existingObject<int>(nullptr, 13476);
  Hndl2 hndl2 = si.inst().template existingObject<int>(nullptr, 13476);
	constexpr auto txn = TRANSACTION(
		remote y = hndl2, remote x = hndl1, x = y, return x)
		::WITH(hndl1,hndl2);
  std::cout << txn << std::endl;
  using ClientTrk = ClientTracker<Label<strong>, Label<causal> >;
  ClientTrk ctrk;
  using res = DECT(txn.run_optimistic<ClientTrk>(ctrk, nullptr, std::declval<typename ClientTrk::connection_references>(),hndl1,hndl2));
	static_assert(std::is_same<res,int>::value);
	txn.run_local(ctrk, hndl1,hndl2);
}
