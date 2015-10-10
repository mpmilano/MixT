
#include "Transaction.hpp"
#include "Ostreams.hpp"

void strong_strong(){
	Handle<Level::strong, HandleAccess::read, int> from;
	Handle<Level::strong, HandleAccess::write, int> to;
	TRANSACTION(
		let_ifValid(dref) = to IN (dref = from)
		);
}

int main() {

}
