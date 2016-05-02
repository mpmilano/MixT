#include <sstream>
#include "MTL.hpp"
#include "SQLStore.hpp"
//#include "TrackerTestingStore.hpp"
#include "RemoteCons.hpp"
#include "Ostreams.hpp"
#include "FinalHeader.hpp"//*/
#include "SerializationMacros.hpp"
#include "Transaction_macros.hpp"
#include "FreeExpr_macros.hpp"
#include "Operate_macros.hpp"

using namespace myria;
using namespace mutils;
using namespace mtl;
using namespace tracker;
using namespace pgsql;


using WeakCons = RemoteCons<int,Level::strong,Level::causal,supports_operation(Increment,SelfType)>;


std::ostream & operator<<(std::ostream &os, const WeakCons& wc){
	return os << "WeakCons{ " << wc.val << ", " << wc.next << "}";
}

int main() {

	try {

    Tracker global_tracker(8765,CacheBehaviors::full);
    RemoteDeserialization_v rdv;
    SQLStore<Level::strong>::SQLInstanceManager sm{global_tracker};
    SQLStore<Level::causal>::SQLInstanceManager cm{global_tracker};
    DeserializationManager dsm{{&sm,&cm}};

	auto log_mgr = build_VMObjectLogger();
	std::unique_ptr<VMObjectLog> log = log_mgr->beginStruct<LoggedStructs::log>();

    SQLStore<Level::strong> &fss = sm.inst_strong(0);
    SQLStore<Level::causal> &fsc = cm.inst_causal(0);
	auto trans = start_transaction(log,global_tracker,fss,fsc);
	auto h = WeakCons::build_list(global_tracker,trans.get(),fss,fsc,12,13,14);

    //std::cout << h.get(global_tracker,nullptr).val.get(global_tracker,nullptr) << std::endl;
	assert(*h.get(global_tracker,trans.get())->val.get(global_tracker,trans.get()) == 14);
	
	trans->full_commit();
	trans.reset();
	
	auto do_test = [&global_tracker,&log](auto h){
		TRANSACTION(log,global_tracker,h,
		let(hd) = h IN (
			WHILE (isValid(hd)) DO(
				print_str("loop"),
				print_str("hd"),
				print(hd),
				let_remote(tmp) = hd IN (
					print_str("tmp"),
					print(tmp),
					let(weak_val) = $(tmp,val) IN (
						do_op(Increment,weak_val)
						),
					hd = $(tmp,next)
					))
			)
			);}; 
	do_test(h);//*/
	
	
	auto trans2 = start_transaction(log,global_tracker,fss,fsc);
	std::cout << *h.get(global_tracker,trans2.get())->val.get(global_tracker,trans2.get()) << std::endl;
	assert(*h.get(global_tracker,trans2.get())->val.get(global_tracker,trans2.get()) == 15);
	trans2->full_commit();
	trans2.reset();
	
	
	auto trans3 = start_transaction(log,global_tracker,fss,fsc);
	auto h2 = WeakCons::build_list(global_tracker,trans3.get(),fss,fsc,18,19,20);
	trans3->full_commit();
	trans3.reset();

	do_test(h2);
	
	auto trans4 = start_transaction(log,global_tracker,fss,fsc);
	std::cout << *h2.get(global_tracker,trans4.get())->val.get(global_tracker,trans4.get()) << std::endl;
	assert(*h2.get(global_tracker,trans4.get())->val.get(global_tracker,trans4.get()) == 21);
	trans4->full_commit();
	trans4.reset();
	
	
	return 0;

	}
	catch(const std::exception & e){
		std::cout << "Program failure: " << e.what() << std::endl;
	}
}
