#include "parse_statements.hpp"
#include "flatten_expressions.hpp"
#include <iostream>
#include "parse_printer.hpp"

using namespace myria;
using namespace mtl;
using namespace parse_phase;

int main(){
  using str = MUTILS_STRING({a.bar().foo(f.isValid(),c,d,a + 3,e,3,a), b = 4, c = a + b});
  constexpr auto tmp = flatten_expressions(parse_statement(str{}));
  std::cout << tmp << std::endl;
}
