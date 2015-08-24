
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
#include "Operate_macros.hpp" //*/
#include "SerializationMacros.hpp"

template<typename T>
FINALIZE_OPERATION(Increment, RemoteObject<T>*);


struct WeakCons :
	public ByteRepresentable<std::pair<ROManager<int>, ROManager<WeakCons> > > {
	Handle<Level::causal, HandleAccess::all, int> val;
	Handle<Level::strong, HandleAccess::all, WeakCons> next;
	
	WeakCons(const decltype(val) &val, const decltype(next) &next)
		:val(val),next(next){}

	DEFAULT_SERIALIZATION_SUPPORT(WeakCons,val,next)
	
};

int main() {

	FileStore<Level::causal> fsc;
	FileStore<Level::strong> fss;
	Handle<Level::strong, HandleAccess::all, WeakCons> h0;
	WeakCons initial{fsc.newObject<HandleAccess::all,int>(12),h0};
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
