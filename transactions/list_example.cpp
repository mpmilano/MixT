#include <sstream>
#include "MTL.hpp"
#include "SQLStore.hpp"
#include "FinalHeader.hpp"
#include "RemoteCons.hpp"
#include "SQLStore.hpp"
#include "Ostreams.hpp"//*/
#include "SerializationMacros.hpp"
#include "Transaction_macros.hpp"
#include "FreeExpr_macros.hpp"
#include "Operate_macros.hpp"

using namespace myria;
using namespace mutils;
using namespace mtl;
using namespace tracker;
using namespace pgsql;

template<Level l, typename T> FINALIZE_OPERATION(Increment, 1, RemoteObject<l, T>* a);

using WeakCons = RemoteCons<int,Level::strong,Level::causal>;


std::ostream & operator<<(std::ostream &os, const WeakCons& wc){
	return os << "WeakCons{ " << wc.val << ", " << wc.next << "}";
}

int main() {

	try {

	Tracker global_tracker(8765);
    RemoteDeserialization_v rdv;
    SQLStore<Level::strong>::SQLInstanceManager sm{global_tracker};
    SQLStore<Level::causal>::SQLInstanceManager cm{global_tracker};
    DeserializationManager dsm{{&sm,&cm}};

    SQLStore<Level::strong> &fss = sm.inst_strong(0);
    SQLStore<Level::causal> &fsc = cm.inst_causal(0);
	auto h = WeakCons::build_list(global_tracker,nullptr,fss,fsc,12,13,14);

    //std::cout << h.get(global_tracker,nullptr).val.get(global_tracker,nullptr) << std::endl;
	assert(h.get(global_tracker,nullptr).val.get(global_tracker,nullptr) == 14);
    return 0;
	auto do_test = [&global_tracker](auto h){
		TRANSACTION(global_tracker,h,
		let(hd) = h IN (
			WHILE (isValid(hd)) DO(
				print_str("loop"),
				print_str("hd"),
				print(hd),
				let_remote(tmp) = hd IN (
					print_str("tmp"),
					print(tmp),
					let_remote(weak_val) = $(tmp,val) IN (
						do_op(Increment,weak_val)
						),
					hd = $(tmp,next)
					))
			)
			);}; //*/
	do_test(h);

	std::cout << h.get(global_tracker,nullptr).val.get(global_tracker,nullptr) << std::endl;
	assert(h.get(global_tracker,nullptr).val.get(global_tracker,nullptr) == 15);

	auto h2 = WeakCons::build_list(global_tracker,nullptr,fss,fsc,18,19,20);
	
	do_test(h2);
	std::cout << h2.get(global_tracker,nullptr).val.get(global_tracker,nullptr) << std::endl;
	assert(h2.get(global_tracker,nullptr).val.get(global_tracker,nullptr) == 21);
	
	return 0;

	}
	catch(const std::exception & e){
		std::cout << "Program failure: " << e.what() << std::endl;
	}
}
