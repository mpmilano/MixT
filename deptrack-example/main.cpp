#include "deptrack.h"
#include <type_traits>
#include <cassert>

using namespace std;

int main(){
	return integral_constant<int, sieve(40)>::value;
	ReadVal(int) a;
	ReadVal(int) b;
	ReadVal(int) c;
	assert (a.id != b.id);
	assert (b.id != c.id);
	assert (a.id != c.id);
}
