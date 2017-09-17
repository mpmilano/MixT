#include "mtl/flatten_expressions.hpp"
#include "mtl/insert_tracking.hpp"
#include "mtl/label_inference.hpp"
#include "mtl/split_phase.hpp"
#include "mtl/transaction.hpp"
#include "mtl/transaction_macros.hpp"
#include "mtl/parse_statements.hpp"
#include "mtl/typecheck_and_label.hpp"
#include "mtl/typecheck_printer.hpp"
#include "mtl/parse_printer.hpp"
#include "mtl/split_printer.hpp"
#include "mtl/relabel.hpp"
#include "mtl/RemoteList.hpp"
#include "mailing_list_example.hpp"
#include <iostream>

using namespace myria;
using namespace mtl;
using namespace parse_phase;
using namespace typecheck_phase;
using namespace label_inference;
using namespace tracking_phase;
using namespace split_phase;
using namespace tracker;

int main() {
	constexpr 
	#include "/tmp/debug"
		type_of_the_day;
	//split_printer<Label<mid> >::print_ast(std::cout, type_of_the_day, "");
	std::cout << type_of_the_day << std::endl;
}
