#include "parse_statements.hpp"
#include "flatten_expressions.hpp"
#include "typecheck_and_label.hpp"
#include <iostream>
#include "typecheck_printer.hpp"

using namespace myria;
using namespace mtl;
using namespace parse_phase;
using namespace typecheck_phase;

int main(){
	//using str = MUTILS_STRING({a.bar().foo(f.isValid(),c,d,a + 3,e,3,a), b = 4, c = a + b});
	using str = MUTILS_STRING({return 3 + 4});
	constexpr auto tmp = typecheck<1,1>(type_environment<Label<top> >{},flatten_expressions(parse_statement(str{})));
  std::cout << tmp << std::endl;
}
