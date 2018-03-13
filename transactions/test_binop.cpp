#include <iostream>
#include "mtl/parse_expressions.hpp"
#include "mtl/parse_printer.hpp"

using namespace myria;
using namespace mutils;
using namespace mtl;
using namespace parse_phase;

int main(){
    using str = MUTILS_STRING(3 + 4 + 5 + 6);
    std::cout << parse_expression(str{}) << std::endl;
}