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


	struct WeakCons :
	 public ByteRepresentable<std::pair<ROManager<int>, ROManager<WeakCons> > > {
		Handle<Level::causal, HandleAccess::all, int> val;
		Handle<Level::strong, HandleAccess::all, WeakCons> next;

		const std::pair<ROManager<int>, ROManager<WeakCons> >&
		manager() const {
			static std::pair<ROManager<int>, ROManager<WeakCons> >
				r{val.manger(),next.manager()};
			return r;
		}

		DEFAULT_SERIALIZE(val,next)

		DEFAULT_DESERIALIZE(WeakCons,val,next)
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
