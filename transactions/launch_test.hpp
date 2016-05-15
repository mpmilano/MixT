#pragma once
#include "ProcessPool.hpp"
#include "ThreadPool.hpp"
#include <iostream>
#include <fstream>
#include <thread>
#include <pqxx/pqxx>
#include "SQLStore.hpp"
#include "FinalHeader.hpp"
#include "Ostreams.hpp"
#include "tuple_extras.hpp"
#include "Basics.hpp"
#include <sys/types.h>
#include <chrono>
#include <cmath>
#include <unistd.h>
#include "ProcessPool.hpp"
#include "Hertz.hpp"
#include "ObjectBuilder.hpp"
#include "test_utils.hpp"
//*/
#include "Transaction_macros.hpp"

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

	struct TrackerMem {
		unique_ptr<VMObjectLogger> log_builder{build_VMObjectLogger()};
		tracker::Tracker trk;
		TrackerMem& tracker_mem(){return *this;}
		
		TrackerMem(int id)
			:trk(id + 1024, tracker::CacheBehaviors::full){
		}
	};
	
	struct SQLMem {
		
		SQLStore<Level::strong>::SQLInstanceManager ss;
		SQLStore<Level::causal>::SQLInstanceManager sc;
		//TrackerTestingStore<Level::strong> ss;
		//TrackerTestingStore<Level::causal> sc;
		DeserializationManager dsm;
		
		SQLMem(TrackerMem& tm)
			:ss(tm.trk),
			 sc(tm.trk),
			 dsm({&ss,&sc}){}
	};

template<typename Mem, typename Arg>
struct PreparedTest{

	using Pool = ThreadPool<SQLMem,Mem,std::string,Arg>;

	//static functions for TaskPool
	static std::string exn_handler(std::exception_ptr eptr);
	
	static void pool_mem_init (int tid, std::shared_ptr<SQLMem> &sqlmem, int memid, std::shared_ptr<Mem> &mem);

	using action_t = typename Pool::action_fp;
	using functor_action_t = typename Pool::action_f;
	
	//members, also for task pool
	const int num_processes;

	//for task pool init
	static std::vector<functor_action_t> convert_vector(const std::vector<action_t>&);

	//task pool
	Pool pool;

	//constructor
	PreparedTest(int num_processes, std::vector<action_t> actions)
		:num_processes(num_processes),
		 pool{pool_mem_init,convert_vector(actions),num_processes,exn_handler}{}

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
	catch (const pqxx::pqxx_exception &e){
		log_messages << "pqxx failure: " << e.base().what() << std::endl;
	}
	catch (const std::exception &e){
		log_messages << "non-pqxx failure: " << e.what() << std::endl;
	}
	catch (...){
		log_messages
			<< "Exception occurred which derived from neither pqxx_exception nor std::exception!"
			<< std::endl;
			}//*/
	return log_messages.str();
}

template<typename Mem,typename Arg>
void PreparedTest<Mem,Arg>::pool_mem_init (int tid, std::shared_ptr<SQLMem>& sqlmem, int memid, std::shared_ptr<Mem> &mem){
	auto pid = memid % (65535 - 1025); //make sure this can be used as a port numbxer
	if (!mem) {
		mem = std::make_shared<Mem>(pid);
	}
	assert(mem);
	if (!sqlmem){
		sqlmem.reset(new SQLMem(mem->tracker_mem()));
	}
	assert(sqlmem);
	auto &ss = sqlmem->ss.inst(get_strong_ip());
	auto &cs = sqlmem->sc.inst(0);
	
	if (!mem->tracker_mem().trk.strongRegistered())
		mem->tracker_mem().trk.registerStore(ss);
	if (!mem->tracker_mem().trk.causalRegistered())
		mem->tracker_mem().trk.registerStore(cs);
	//I'm assuming that pid won't get larger than the number of allowable ports...
	assert(pid + 1024 < 49151);
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
					catch (const pqxx::pqxx_exception& e){
						ss << e.base().what() << endl;
					}
					catch (...){
						std::exception_ptr p = std::current_exception();
						ss <<(p ? p.__cxa_exception_type()->name() : "null") << std::endl;

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
