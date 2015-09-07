

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
#include "SQLStore.hpp"
#include "FinalHeader.hpp" //*/
#include "SerializationMacros.hpp"


template<typename T>
FINALIZE_OPERATION(Increment, RemoteObject<T>*);


struct WeakCons :
	public ByteRepresentable {
	using WeakCons_r = Handle<Level::strong, HandleAccess::all, WeakCons>;
	using WeakCons_v = Handle<Level::causal, HandleAccess::all, int>;
	
	WeakCons_v val;
	WeakCons_r next;
	
	WeakCons(const decltype(val) *val, const decltype(next) *next)
		:val(*val),next(*next){
		delete val; delete next;
	}

	WeakCons(const decltype(val) &val, const decltype(next) &next)
		:val(val),next(next){}

	template<typename Strong, typename Causal, typename... Args>
	static WeakCons_r build_list(Strong& strong, Causal& causal, const Args & ... args){
		auto tpl = std::make_tuple(args...);
		return fold(tpl,[&](const int &e, const auto & acc){
				WeakCons initial{causal.template newObject<HandleAccess::all,int>(e),acc};
				return strong.template newObject<HandleAccess::all,WeakCons>(initial);
			},WeakCons_r());
	}

	DEFAULT_SERIALIZATION_SUPPORT(WeakCons,val,next)
	
};

int main() {

	auto &fsc = 
		FileStore<Level::causal>::filestore_instance();
	auto& fss =
		SQLStore::inst();
	auto h = WeakCons::build_list(fss,fsc,12,13,14);
	
	assert(h.get().val.get() == 12);

	TRANSACTION(
		let_mutable(hd) = h IN (
			WHILE (isValid(hd)) DO(
				let_ifValid(tmp) = hd IN (
					let_ifValid(weak_val) = msg(tmp,val)
					  IN (do_op(Increment,weak_val)),
					hd = msg(tmp,next)
					)
				)
			)
		);
	
	assert(h.get().val.get() == 13);
	
	return 0;
}
