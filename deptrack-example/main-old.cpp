#include "deptrack.h"
#include <type_traits>
#include <cassert>
#include <iostream>

using namespace std;

int main(){

	ReadVal(int) a(1);
	ReadVal(int) b(2);
	ReadVal(int) b2(123);
	ReadVal(int) c(3);
	assert (gcd(35,49) == 7);
	assert (a.id() != b.id());
	assert (b.id() != c.id());
	assert (a.id() != c.id());

	WriteVal<int,a.id(),b.id()> sink;

	a.display();
	b.display();
	c.display();

	assert (gcd(a.id(), b.id()) == 1);
	assert (gcd(c.id(), b.id()) == 1);
	assert (gcd(b.id(), c.id()) == 1);
	assert (gcd(a.id(), c.id()) == 1);

	{ 
		auto a_ = a + c;
		auto a = a_;

		a.display();
		

		
		// if (a) {
		// 	b += c; assert(false);
		// }
		// else {
		// 	c += b; assert(false);
		// }
		
		// typedef decltype(a.touch(std::move(b))) A;
		// typedef decltype(a.touch(std::move(c))) B;
		// void (*f)(A, B) = [](A b, B c){ b += c; assert(false);};
		// void (*g)(A, B) = [](A b, B c){ c += b; assert(false);};
		// a.ifTrue(f,g,b,c);
		
		TIF3((a == b), 
		     //then
		     {
			     auto b_ = b + a + c;
			     cout << "true branch" << endl;
			     b_.display();
			     b_.displaySources();
		     }, 
		     //else
		     {
			     c.displaySources();
			     auto c_ = c + b;
			     cout << "false branch" << endl;
			     c_.display();
			     c_.displaySources();
		     }, 
		     //params
		     a, b, c);
		
		//fails - incorporates c, which wasn't on allowed list
		//sink.put(a);
		
		//succeeds - allows a and b
		sink.put(b);

		
		
		cout <<  integral_constant<int, sieve(80)>::value << endl;
		return 0;
	}
}
