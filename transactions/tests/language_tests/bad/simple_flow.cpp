
#include "Transaction.hpp"
#include "Ostreams.hpp"

using namespace myria;

void weak_strong() {
	Handle<Level::causal, HandleAccess::read, int> from;
	Handle<Level::strong, HandleAccess::write, int> to;
	TRANSACTION(
		let_remote(dref) = to IN (dref = from)
		);

}

int main() {
	weak_strong();
}
