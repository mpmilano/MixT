#include "deptrack.h"
#include <type_traits>
#include <cassert>
#include <iostream>

#define lambda(args...)

using namespace std;

int main(){

	ReadVal(int) a(1);
	ReadVal(int) b(2);
	ReadVal(int) c(3);
	assert (gcd(35,49) == 7);
	assert (a.id() != b.id());
	assert (b.id() != c.id());
	assert (a.id() != c.id());

	a.display();
	b.display();
	c.display();

	a += c;

	a.display();

	assert (gcd(a.id(), b.id()) == 1);
	assert (gcd(c.id(), b.id()) == 1);
	assert (gcd(b.id(), c.id()) == 1);
	assert (gcd(a.id(), c.id()) == 1);

	typedef decltype(a.touch(std::move(b))) A;
	typedef decltype(a.touch(std::move(c))) B;
	void (*f)(A, B) = [](A b, B c){ assert(false);};
	void (*g)(A, B) = [](A b, B c){ assert(false);};
	a.ifTrue(f,g,b,c);

	cout <<  integral_constant<int, sieve(80)>::value << endl;
	return 0;
}
