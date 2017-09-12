#include "mtl/parse_statements.hpp"
#include <iostream>
#include "mtl/parse_printer.hpp"

using namespace myria;
using namespace mtl;
using namespace parse_phase;

int main(){
	using str = MUTILS_STRING(var a = 3, var b = 5, while (true) {remote c  = a, var d = 3, d = d+4});
	std::cout << parse_statement(str{}) << std::endl;
}
