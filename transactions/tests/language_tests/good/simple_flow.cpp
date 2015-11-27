
#include "Transaction.hpp"
#include "Ostreams.hpp"
using namespace myria;

void strong_strong(){
	Handle<Level::strong, HandleAccess::read, int> from;
	Handle<Level::strong, HandleAccess::write, int> to;
	TRANSACTION(
		let_remote(dref) = to IN (dref = from)
		);
}

void strong_weak() {
	Handle<Level::strong, HandleAccess::read, int> from;
	Handle<Level::causal, HandleAccess::write, int> to;
	TRANSACTION(
		let_remote(dref) = to IN (dref = from)
		);

}

void weak_weak() {
	Handle<Level::causal, HandleAccess::read, int> from;
	Handle<Level::causal, HandleAccess::write, int> to;
	TRANSACTION(
		let_remote(dref) = to IN (dref = from)
		);
}


int main() {
	strong_strong();
	strong_weak();
	weak_weak();
}
