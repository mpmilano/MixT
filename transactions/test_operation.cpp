#include "parse_statements.hpp"
#include "flatten_expressions.hpp"
#include "typecheck_and_label.hpp"
#include <iostream>
#include "typecheck_printer.hpp"
#include "testing_store/TestingStore.hpp"

using namespace myria;
using namespace mtl;
using namespace parse_phase;
using namespace typecheck_phase;
using namespace testing_store;

int main(){
	using Store = TestingStore<Label<top> >;
	using int_handle = typename Store::template TestingHandle<int>;
	using int_handle_handle = typename Store::template TestingHandle<int_handle>;
	int_handle_handle a;
	using str = MUTILS_STRING({(*a).noop()});
	constexpr auto parsed = flatten_expressions(parse_statement(str{}));
	constexpr auto tmp = typecheck<1,1>(type_environment<Label<top>,
										type_binding<MUTILS_STRING(a),DECT(a),Label<top>, type_location::local > >{},parsed);
	std::cout << tmp << std::endl;
}
