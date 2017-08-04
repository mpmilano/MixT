#include "parse_statements.hpp"
#include "flatten_expressions.hpp"
#include "typecheck_and_label.hpp"
#include <iostream>
#include "typecheck_printer.hpp"
#include "testing_store/TestingStore.hpp"
#include "mtl/label_inference.hpp"
#include "mtl/insert_tracking.hpp"
#include "mtl/split_phase.hpp"

using namespace myria;
using namespace mtl;
using namespace parse_phase;
using namespace typecheck_phase;
using namespace testing_store;
using namespace label_inference;
using namespace tracking_phase;
using namespace split_phase;

int main(){
	using Store = TestingStore<Label<top> >;
	using int_handle = typename Store::template TestingHandle<int>;
	using int_handle_handle = typename Store::template TestingHandle<int_handle>;
	int_handle_handle a;
	using str = MUTILS_STRING({(*a).noop(1,2,3,4)});
	constexpr auto parsed = flatten_expressions(parse_statement(str{}));
	constexpr auto tmp = typecheck<1,1>(type_environment<Label<top>,
										type_binding<MUTILS_STRING(a),DECT(a),Label<top>, type_location::local > >{},parsed);
	using inferred_t = DECT(infer_labels(tmp));
	std::cout << tmp << std::endl;
	std::cout << inferred_t{} << std::endl;
	using tracked_t = DECT(insert_tracking_begin(inferred_t{}));
	auto split = split_computation<tracked_t, type_binding<MUTILS_STRING(a),DECT(a),Label<top>, type_location::local > >();
	std::cout << split << std::endl;
}
