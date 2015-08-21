#include "IfBuilder.hpp"
#include "TransactionBuilder.hpp"
#include "Transaction.hpp"
#include "BaseCS.hpp"
#include "FileStore.hpp"
#include "Operate.hpp"
#include "TempBuilder.hpp"
#include "FreeExpr.hpp"
#include "Transaction_macros.hpp"

template<typename T>
FINALIZE_OPERATION(Increment, RemoteObject<T>*)


struct WeakCons {
	Handle<Level::causal, HandleAccess::all, int> val;
	Handle<Level::strong, HandleAccess::all, WeakCons> next;
};

int main() {

	int a = 3;

	Handle<Level::strong, HandleAccess::all, WeakCons> h;

	TRANSACTION(
		let_mutable(hd) = h IN (
			WHILE (isValid(hd)) DO(
				let_ifValid(tmp) = hd IN (
					do_op(Increment,msg(hd,val)),
					hd = msg(hd,next)
					)
				)
			)
		)
	
	return 0;
}
