#include "Transaction.hpp"
#include "Ostreams.hpp"

int main(){
	Handle<Level::strong, HandleAccess::all, int> h1;
	TRANSACTION(let_ifValid(x) = h1 IN (x = h1));
}
