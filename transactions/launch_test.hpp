#pragma once
#include "GloballyBackedExecutor.hpp"
#include <iostream>
#include <fstream>
#include <thread>
#include "SQLStore.hpp"
#include "FinalHeader.hpp"
#include "Ostreams.hpp"
#include "tuple_extras.hpp"
#include "Basics.hpp"
#include <sys/types.h>
#include <chrono>
#include <cmath>
#include <unistd.h>
#include "Hertz.hpp"
#include "ObjectBuilder.hpp"
#include "test_utils.hpp"
//*/
#include "Transaction_macros.hpp"
#include "abiutils.hpp"

//The "new" vm_main


constexpr int my_unique_id = int{IP_QUAD};

using namespace std;
using namespace chrono;
using namespace mutils;
using namespace myria;
using namespace mtl;
using namespace pgsql;

namespace mutils{

namespace {
	constexpr unsigned long long bigprime_lin =
#include "big_prime"
		;
}

	struct Mem {
		unique_ptr<VMObjectLogger> log_builder{build_VMObjectLogger()};
		tracker::Tracker trk;
		const int memid;
		struct Continue_build {
			SQLStore<Level::strong>::SQLInstanceManager ss;
			SQLStore<Level::causal>::SQLInstanceManager sc;
			DeserializationManager dsm;
			
			Continue_build(Mem& super, SQLConnectionPool<Level::strong>& strong_p, SQLConnectionPool<Level::causal>& causal_p)
				:ss(super.trk,strong_p),
				 sc(super.trk,causal_p),
				 dsm({&ss,&sc}){
				auto pid = super.memid % (65535 - 1025); //make sure this can be used as a port numbxer
				auto &ss = this->ss.inst();
				auto &cs = sc.inst();
				if (!super.trk.strongRegistered())
					super.trk.registerStore(ss);
				if (!super.trk.causalRegistered())
					super.trk.registerStore(cs);
				//I'm assuming that pid won't get larger than the number of allowable ports...
				assert(pid + 1024 < 49151);
			}
		};
		std::unique_ptr<Continue_build> i;
		
		Mem& tracker_mem(){return *this;}
		
		Mem(int memid)
			:trk(memid + 1024, tracker::CacheBehaviors::full),
			 memid(memid) {}
		Mem(const Mem&) = delete;
	};
	

template<typename Mem, typename Arg>
struct PreparedTest{

	using Pool = GloballyBackedExecutor<Mem,std::string,Arg>;

	//static functions for TaskPool
	static std::string exn_handler(std::exception_ptr eptr);

	SQLConnectionPool<Level::strong> strong;
	SQLConnectionPool<Level::causal> causal;
	void pool_mem_init (Mem& m){
		m.i.reset(new typename Mem::Continue_build(m,strong,causal));
	}

	using action_t = typename Pool::action_fp;
	using functor_action_t = typename Pool::action_f;
	
	//for task pool init
	static std::vector<functor_action_t> convert_vector(const std::vector<action_t>&);

	//task pool
	Pool pool;

	//constructor
	PreparedTest(std::vector<action_t> actions)
		:pool{[this](int,Mem& m){return this->pool_mem_init(m);},
			convert_vector(actions),
				exn_handler}{}

	//runs the main test loop, given a function which return true when
	//the test should stop, and a function that returns (which-task, what-arg-for-task)
	template<typename Meta>
	std::string run_tests(Meta& meta, bool (*stop) (Meta&, Pool&),
						  std::pair<int,Arg> (*choose_action) (Meta&, Pool&),
						  milliseconds (*delay) (Meta&, Pool&));
};



//implementations 



//static functions for TaskPool
template<typename Mem, typename Arg>
std::string PreparedTest<Mem,Arg>::exn_handler(std::exception_ptr eptr){
	std::stringstream log_messages;
	try {
		assert(eptr);
		std::rethrow_exception(eptr);
	}
	catch (const std::exception &e){
		log_messages << "failure: " << e.what() << std::endl;
	}
	catch (...){
		log_messages
			<< "Exception occurred which derived from not std::exception!"
			<< std::endl;
			}//*/
	return log_messages.str();
}

template<typename Mem,typename Arg>
std::vector<typename PreparedTest<Mem,Arg>::functor_action_t>
PreparedTest<Mem,Arg>::convert_vector(const std::vector<action_t>& src){
	std::vector<functor_action_t> ret;
	for (auto &f : src) ret.push_back(f);
	return ret;
}

	
//runs the main test loop, given a function which return true when
//the test should stop, and a function that returns (which-task, what-arg-for-task)
template<typename Mem,typename Arg> template<typename Meta>
std::string PreparedTest<Mem,Arg>::run_tests(Meta& meta, bool (*stop) (Meta&, Pool&),
											 std::pair<int,Arg> (*choose_action) (Meta&, Pool&),
											 milliseconds (*delay) (Meta&, Pool&)){
	
	using future_list = std::list<std::future<std::unique_ptr<std::string> > >;
	std::unique_ptr<future_list> futures{new future_list()};
	
	while(!stop(meta,pool)){
		std::this_thread::sleep_for(delay(meta,pool));
		auto decision = choose_action(meta,pool);
		futures->emplace_back(pool.launch(decision.first,decision.second) );
	}
	
	std::stringstream ss;
	while (!futures->empty()){
		std::unique_ptr<future_list> new_futures{new future_list()};
		for (auto &f : *futures){
			if (f.valid()){
				if (f.wait_for(1ms) != future_status::timeout){
					try {
						auto strp = f.get();
						if (strp){
							ss << *strp << endl;
						}
					}
					catch (const std::exception& e){
						ss << e.what() << endl;
					}
					catch (...){
						std::exception_ptr p = std::current_exception();
						ss << "threw this: ";
						if (p){
							ss << exn_typename(p) << std::endl;
						}
						else {
							ss << "null" << std::endl;
						}
					}
					
				}
				else {
					new_futures->push_back(std::move(f));
				}
			}
		}
		futures = std::move(new_futures);
	}
	return ss.str();
}

}
