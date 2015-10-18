
#include <sstream>
#include "IfBuilder.hpp"
#include "TransactionBuilder.hpp"
#include "Transaction.hpp"
#include "BaseCS.hpp"
#include "FileStore.hpp"
#include "Operate.hpp"
#include "TempBuilder.hpp"
#include "FreeExpr.hpp"
#include "Print.hpp"
#include "Massert.hpp"
#include "SQLStore.hpp"
#include "FinalHeader.hpp" //*/
#include "RemoteCons.hpp"
#include "SQLStore.hpp"
#include "SerializationMacros.hpp"
#include "Transaction_macros.hpp"
#include "FreeExpr_macros.hpp"
#include "Operate_macros.hpp"



template<typename T>
FINALIZE_OPERATION(Increment, RemoteObject<T>*);

using WeakCons = RemoteCons<int,Level::strong,Level::causal>;


std::ostream & operator<<(std::ostream &os, const WeakCons& wc){
	return os << "WeakCons{ " << wc.val << ", " << wc.next << "}";
}


int main() {

	auto &fsc = 
		FileStore<Level::causal>::filestore_instance();
	auto& fss =
		SQLStore::inst();
	auto h = WeakCons::build_list(fss,fsc,12,13,14);

	assert(h.get().val.get() == 14);

		TRANSACTION(
			let_ifValid(zero) = 0 IN (
				let_mutable(hd) = h IN (
			WHILE (isValid(hd)) DO(
				print_str("loop"),
				print_str("hd"),
				print(hd),
				let_ifValid(tmp) = hd IN (
					print_str("tmp"),
					print(tmp),
					let_ifValid(weak_val) = $(tmp,val) IN (
						do_op(Increment,weak_val)
						),
					hd = $(tmp,next)
					))
			))
			); //*/

		std::cout << h.get().val.get() << std::endl;
		assert(h.get().val.get() == 15);
		return 0;
}
