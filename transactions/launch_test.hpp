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
			SQLStore<Label<strong> >::SQLInstanceManager ss;
			SQLStore<Label<causal> >::SQLInstanceManager sc;
			DeserializationManager dsm;
			simple_rpc::connection strong_connection;
			simple_rpc::connection causal_connection;
			
			Continue_build(Mem& , SQLConnectionPool<Label<strong> >& strong_p, SQLConnectionPool<Label<causal> >& causal_p,
										 simple_rpc::connection strong_connection,	simple_rpc::connection causal_connection)
				:ss(strong_p),
				 sc(causal_p),
				 dsm({&ss,&sc}),
				 strong_connection(std::move(strong_connection)),
				 causal_connection(std::move(causal_connection)){
			}
		};
		std::unique_ptr<Continue_build> i;
		
		Mem& tracker_mem(){return *this;}
		DeserializationManager* dsm() {
			assert(i);
			return &i->dsm;
		}
		
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
	SQLConnectionPool<Label<strong> > strong;
	SQLConnectionPool<Label<causal> > causal;
	simple_rpc::connections strong_connections;
	simple_rpc::connections causal_connections;
	void pool_mem_init (Mem& m){
		m.i.reset(new typename Mem::Continue_build(m,strong,causal,strong_connections.spawn(),	causal_connections.spawn()));
	}

	using action_t = typename Pool::action_fp;
	using functor_action_t = typename Pool::action_f;
	
	//for task pool init
	static std::vector<functor_action_t> convert_vector(const std::vector<action_t>&);

	//task pool
	Pool pool;

	//constructor
	PreparedTest(std::vector<action_t> actions, int strong_ip, int strong_port, int strong_max, int causal_ip, int causal_port, int causal_max)
		:strong_connections(strong_ip,strong_port,strong_max),
		 causal_connections(causal_ip,causal_port, causal_max),
		 pool{[this](int,Mem& m){return this->pool_mem_init(m);},
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

	std::cout << "Launches Complete.  Waiting for tasks to terminate: " << std::endl;
	
	std::stringstream ss;
	std::size_t old_size = 0;
	for (unsigned int timeout = 0; timeout < 10 && !futures->empty() ; ){
		if (old_size == futures->size()) ++timeout;
		old_size = futures->size();
		std::cout << futures->size() << " tasks remain" << std::endl;
		std::unique_ptr<future_list> new_futures{new future_list()};
		for (auto &f : *futures){
			if (f.valid()){
				if (f.wait_for(100us) != future_status::timeout){
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
		sleep(1);
	}
	auto strong_info = strong.acquire()->collect_machine_stats();
	auto causal_info = causal.acquire()->collect_machine_stats();
	std::cout << "memory usage on the remote hosts: " << std::endl;
	std::cout << "causal: " << causal_info.totalram - causal_info.freeram << std::endl;
	std::cout << "strong: " << strong_info.totalram - strong_info.freeram << std::endl;
	return ss.str();
}

}
