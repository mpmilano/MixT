#include "Transactions.hpp"

int main(){
	ConStatement<strong> a("a");
	ConStatement<weak> b("b");
	
	Seq<2,1> f(a);
	f,b;
}
