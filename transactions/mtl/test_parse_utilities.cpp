#include "mlt/parse_utilities.hpp"

using namespace myria;
using namespace mtl;
using namespace parse_utilities;

int main(){
	return is_paren_group<'(', ')', 'a', '(', ')'>();
}
