#include "Transaction.hpp"
#include "Ostreams.hpp"
#include <string>

using namespace std;
using namespace myria;

int main(){
	Handle<Level::strong, HandleAccess::all, int> h1;
	TRANSACTION(let_remote(x) = h1 IN (x = string("death!")));
}
