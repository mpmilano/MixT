#include "parse_statements.hpp"
#include "flatten_expressions.hpp"
#include "typecheck_and_label.hpp"
#include <iostream>
#include "typecheck_printer.hpp"
#include "testing_store/TestingStore.hpp"
#include "mtl/label_inference.hpp"
#include "mtl/insert_tracking.hpp"
#include "mtl/split_phase.hpp"
#include "mtl/transaction.hpp"
#include "mtl/transaction_macros.hpp"

using namespace myria;
using namespace mtl;
using namespace parse_phase;
using namespace typecheck_phase;
using namespace testing_store;
using namespace label_inference;
using namespace tracking_phase;
using namespace split_phase;
using namespace tracker;

int main(){
	using Store = TestingStore<Label<top> >;
	using int_handle = typename Store::template TestingHandle<int>;
	using int_handle_handle = typename Store::template TestingHandle<int_handle>;
	Store store;
	constexpr int a_name = 43;
	constexpr int int_name = 23;
	int_handle_handle a = store.template newObject<int_handle>(nullptr,a_name,store.template newObject<int>(nullptr,int_name,23));
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
	using ClientTrk = ClientTracker<>;
	ClientTrk ct;
	TRANSACTION((*a).noop(1,2,3,4))::WITH(a).run_local(ct,a);
}
