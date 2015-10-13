#include "Transaction.hpp"
#include "Ostreams.hpp"
#include <string>

using namespace std;

int main(){
	Handle<Level::strong, HandleAccess::all, int> h1;
	TRANSACTION(let_ifValid(x) = h1 IN (x = string("death!")));
}
