#include "IfBuilder.hpp"
#include "TransactionBuilder.hpp"
#include "Transaction.hpp"
#include "BaseCS.hpp"
#include "FileStore.hpp"
#include "Operate.hpp"
#include "TempBuilder.hpp"
#include "FreeExpr.hpp"
#include "Transaction_macros.hpp"
#include "FreeExpr_macros.hpp"
#include "Operate_macros.hpp"

template<typename T>
FINALIZE_OPERATION(Increment, RemoteObject<T>*)


struct WeakCons {
	Handle<Level::causal, HandleAccess::all, int> val;
	Handle<Level::strong, HandleAccess::all, WeakCons> next;
	
	void* to_bytes() const {
		void a[val.to_bytes_size()] = val.to_bytes();
		assert(sizeof(a) == val.to_bytes_size());
		void b[next.to_bytes_size()] = next.to_bytes();
		void* both = malloc(sizeof(a) + sizeof(b));
		memcpy(both,     a, sizeof(a));
		memcpy(both + 4, b, sizeof(b)); 
		return both;
	}

	template<typename RObject>
	static WeakCons from_bytes(void *v) {
		return WeakCons{Handle<Level::causal, HandleAccess::all, int>::
				template from_bytes<RObject>(v[0]),
				Handle<Level::strong, HandleAccess::all, WeakCons>::
				template from_bytes<RObject>(arr->at(2))};
	}
};

int main() {

	FileStore<Level::causal> fsc;
	FileStore<Level::strong> fss;
	WeakCons initial{fsc.newObject<HandleAccess::all,int>(12)};
	Handle<Level::strong, HandleAccess::all, WeakCons> h =
		fss.newObject<HandleAccess::all, WeakCons>(initial);
	


	TRANSACTION(
		let_mutable(hd) = h IN (
			WHILE (isValid(hd)) DO(
				let_ifValid(tmp) = hd IN (
					do_op(Increment,msg(tmp,val)),
					hd = msg(tmp,next)
					)
				)
			)
		)
	
	return 0;
}
