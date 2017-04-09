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

#define txn_text let remote y = hndl2 in { let remote x = hndl1 in { x = y, return x } } 

int main(){
  Hndl1 hndl1;
  Hndl2 hndl2;
  using txn_str =
    MUTILS_STRING(txn_text);
  constexpr auto constraints =
    minimize_constraints(collapse_constraints(collect_constraints(Label<top>{},typecheck<1,1>(type_environment<Label<top>,
						    type_binding<MUTILS_STRING(hndl1),Hndl1,Label<top>,type_location::local >,
						    type_binding<MUTILS_STRING(hndl2),Hndl2,Label<top>,type_location::local >
											      >{},flatten_expressions(parse_statement(txn_str{}))))));
	constexpr auto txn = TRANSACTION(0,txn_text)::WITH(hndl1,hndl2);

  std::cout << constraints << std::endl;
	std::cout << txn << std::endl;
}
