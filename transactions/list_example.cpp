
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
#include "SerializationMacros.hpp"
#include "Transaction_macros.hpp"
#include "FreeExpr_macros.hpp"
#include "Operate_macros.hpp"



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

	bool operator==(WeakCons wc) {
		std::stringstream ss1;
		std::stringstream ss2;
		ss1 << val << next;
		ss2 << wc.val << wc.next;
		return ss1.str() == ss2.str();
	}

	bool equals(WeakCons wc) const {
		WeakCons copy = *this;
		return copy == wc;
	}

	DEFAULT_SERIALIZATION_SUPPORT(WeakCons,val,next)	
};

std::ostream & operator<<(std::ostream &os, const WeakCons& wc){
	return os << "WeakCons{ " << wc.val << ", " << wc.next << "}";
}


int main() {

	auto &fsc = 
		FileStore<Level::causal>::filestore_instance();
	auto& fss =
		FileStore<Level::strong>::filestore_instance();
	auto h = WeakCons::build_list(fss,fsc,12,13,14);

	std::cout << h.get().val.get() << std::endl;
	assert(h.get().val.get() == 14);

	/*
	TRANSACTION(
		let_mutable(bound) = 0 IN (
		let_mutable(hd) = h IN (
			WHILE (isValid(hd) && (!(bound == 10))) DO(
				print_str("hd"),
				print(hd),
				bound = bound + 1,
				print_str("bound: "),
				print(bound),
				let_mutable(tmp2) = hd IN (
				let_ifValid(tmp) = hd IN (
					print_str("tmp2, tmp and hd should be the same:"),
					print(tmp2),
					print(tmp),
					print(hd),
					massert(free_expr(bool,tmp,tmp2,tmp.equals(tmp2))),
					let_ifValid(weak_val) = msg(tmp,val) IN (
						print_str("weak_val: "),
						print(dref(weak_val)),
						do_op(Increment,weak_val)),
					hd = msg(tmp,next),
					print_str("tmp, hd, h (hd should be shorter)"),
					print(tmp),
					print(hd),
					print(h)
					))
				)
			)
			));
	//*/ 
		TRANSACTION(
		let_mutable(bound) = 0 IN (
		let_mutable(hd) = h IN (
			WHILE (isValid(hd) && (!(bound == 10))) DO(
				let_ifValid(tmp) = hd IN (
					let_ifValid(weak_val) = msg(tmp,val) IN (
						do_op(Increment,weak_val))
					hd = msg(tmp,next)
					))
				)
			)
			); //*/


	std::cout << h.get().val.get() << std::endl;
	assert(h.get().val.get() == 15);
	
	return 0;
}
