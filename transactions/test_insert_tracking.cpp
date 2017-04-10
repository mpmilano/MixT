#include "SQLStore.hpp"
#include "transaction.hpp"
#include "transaction_macros.hpp"
#include "FinalHeader.hpp"
#include "CTString.hpp"
#include "CTString_macro.hpp"
#include "common_strings.hpp"
#include "AST_parse.hpp"
#include "AST_typecheck.hpp"
#include "parse_statements.hpp"
#include "parse_expressions.hpp"
#include "typecheck_and_label.hpp"
#include "utils.hpp"
#include "flatten_expressions.hpp"
#include "split_phase.hpp"
#include "typecheck_printer.hpp"
#include "label_inference.hpp"
#include "parse_printer.hpp"
#include "split_printer.hpp"
#include "interp.hpp"
#include "transaction_macros.hpp"

using namespace myria;
using namespace server;
using namespace pgsql;
using namespace mtl;
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
  Hndl1 hndl1 = ci.inst().template existingObject<int>(13476);
  Hndl2 hndl2 = si.inst().template existingObject<int>(13476);
	constexpr auto txn = TRANSACTION(
		0,
		let remote y = hndl2 in { let remote x = hndl1 in { x = y, return x } } )
		::WITH(hndl1,hndl2);
  std::cout << txn << std::endl;
	using res = DECT(txn.run_optimistic(nullptr, std::declval<mutils::connection&>(), std::declval<mutils::connection&>(),hndl1,hndl2));
	static_assert(std::is_same<res,int>::value);
	txn.run_local(hndl1,hndl2);
}
