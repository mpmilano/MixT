
#include "ProcessPool.hpp"
#include "ThreadPool.hpp"
#include <iostream>
#include <fstream>
#include <thread>
#include <pqxx/pqxx>
#include "SQLStore.hpp"
#include "TrackerTestingStore.hpp"
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
#include "launch_test.hpp"
#include "TrackerTestingStore.hpp"//*/
#include "Transaction_macros.hpp"

using namespace std;
using namespace chrono;
using namespace mutils;
using namespace myria;
using namespace mtl;
using namespace pgsql;


#ifdef NO_USE_STRONG
constexpr bool causal_enabled = true;
#endif
#ifdef USE_STRONG
constexpr bool causal_enabled = false;
#endif

constexpr int num_processes = 50;
static_assert(num_processes <= 100,"Error: you are at risk of too many open files");
constexpr auto arrival_rate = 800_Hz;
constexpr auto actual_arrival_rate = arrival_rate * as_hertz(1 + int{concurrencySetting});

const auto log_name = [](){
	auto pid = getpid();
	return std::string("/tmp/MyriaStore-output-") + std::to_string(pid);
}();
ofstream logFile;

constexpr int name_max = 478446;

int get_name_read(double alpha){
	constexpr int max = name_max;
	auto ret = get_zipfian_value(max,alpha);
	if (ret > (max + 14)) {
		std::cerr << "Name out of range! Trying again" << std::endl;
		return get_name_read(alpha);
	}
	else return ret + 14;
}

int get_name_write(){
        return int_rand() % 478446;
}

const int mod_constant = 50;

int main(){

	std::cout << "In configuration; " << (causal_enabled ? "with causal" : " with only strong" ) << std::endl;
	static const int ip = get_strong_ip();
	logFile.open(log_name);
	//auto prof = VMProfiler::startProfiling();
	//auto longpause = prof->pause();
	auto logger = build_VMObjectLogger();
	auto global_log = logger->template beginStruct<LoggedStructs::globals>();
	global_log->addField(GlobalsFields::request_frequency,actual_arrival_rate);
	std::cout << "hello world from VM "<< my_unique_id << " in group " << CAUSAL_GROUP << std::endl;
	std::cout << "connecting to " << string_of_ip(ip) << std::endl;
	
	/*
//init
        SQLStore<Level::strong> &strong = SQLStore<Level::strong>::inst(ip);
	SQLStore<Level::causal> &causal = SQLStore<Level::causal>::inst(0);
	for (int i = 40743; i < std::numeric_limits<int>::max();++i){
		std::cout << i << std::endl;
		if (!strong.exists(i))
			strong.template newObject<HandleAccess::all,int>(i,0);
		if (!causal.exists(i))
		causal.template newObject<HandleAccess::all,int>(i,0);
	}
	exit(0); */

	//tracker testing init
	
        if (false){
		using namespace ::myria::testing;
		unique_ptr<VMObjectLogger> log_builder{build_VMObjectLogger()};
		tracker::Tracker trk{1025,tracker::CacheBehaviors::none};
		TrackerTestingStore<Level::strong> strong{trk};
		TrackerTestingStore<Level::causal> causal{trk};
                auto log = log_builder->template beginStruct<LoggedStructs::log>();
                assert(log);
		auto ctx = start_transaction(std::move(log),trk,strong,causal);
                assert(ctx->trackingContext);
                assert(ctx->trackingContext->logger);
		for (int i = 0; i <= name_max; ++i){
			if (!strong.exists(i))
				strong.template newObject<HandleAccess::all,int>(trk,ctx.get(),i,0);
			if (!causal.exists(i))
				causal.template newObject<HandleAccess::all,int>(trk,ctx.get(),i,0);
		}

		strong.template newObject<HandleAccess::all,std::array<int,4> >(
			trk,ctx.get(),bigprime_lin,{{0,0,0,0}});
		
		ctx->full_commit();
		
		std::cout << "trackertesting init complete" << std::endl;
	}//*/


	using namespace testing;

	struct PoolFunStruct {
		static std::string pool_fun(std::shared_ptr<Remember> mem, int i, unsigned long long _start_time){
			assert(mem);
			unique_ptr<VMObjectLog> log_messages{
				mem->log_builder->template beginStruct<LoggedStructs::log>().release()};
			microseconds start_time(_start_time);
			auto run_time = elapsed_time();
			
			log_messages->addField(LogFields::submit_time,
								   duration_cast<milliseconds>(start_time).count());
			log_messages->addField(LogFields::run_time,
								   duration_cast<milliseconds>(run_time).count());
			//std::cout << "launching task on pid " << pid << std::endl;
			//AtScopeEnd em{[pid](){std::cout << "finishing task on pid " << pid << std::endl;}};
			try{
				std::string str = [&](){
					SQLStore<Level::strong> &strong = mem->ss.inst_strong(ip);
					SQLStore<Level::causal> &causal = mem->sc.inst_causal(0);
					//auto &strong = mem->ss;
					//auto &causal = mem->sc;
					auto &trk = mem->trk;
					assert(!strong.in_transaction());
					assert(!causal.in_transaction());
					assert(!trk.get_StrongStore().in_transaction());
					
					bool do_write = (int_rand() % mod_constant)==0;
					auto name = (do_write ? get_name_write() : get_name_read(0.5));
					
					auto test_fun = [&](auto hndl){
						
						for(int tmp2 = 0; tmp2 < 10; ++tmp2){
							try{
								if (do_write){
									TRANSACTION(log_messages,trk,hndl,
												do_op<RegisteredOperations::Increment>(hndl)
										)//*/
										log_messages->addField(
											LogFields::is_write,true);
								}
								else {
									assert(true);
									TRANSACTION(log_messages,
												trk,hndl,
												let_remote(tmp) = hndl IN(mtl_ignore($(tmp)))
										);
									log_messages->addField(
										LogFields::is_read,true);
								}
								auto end = elapsed_time();
								log_messages->addField(LogFields::done_time,
													   duration_cast<milliseconds>(end).count());
								log_messages->addField(LogFields::is_serialization_error,false);
								break;
							}
							catch(const SerializationFailure &r){
								auto end = elapsed_time();
								log_messages->addField(LogFields::done_time,
													   duration_cast<milliseconds>(end).count());
								log_messages->addField(LogFields::is_serialization_error,true);
								continue;
							}
					}
						return log_messages->single();
					};
					if (better_rand() > .7 || !causal_enabled){
						auto hndl = strong.template
							existingObject<HandleAccess::all,int>(log_messages,name);
						return test_fun(hndl);
					}
					else {
						auto hndl = causal.template
						existingObject<HandleAccess::all,int>(log_messages,name);
						return test_fun(hndl);
					}
				}();
				//std::cout << str << std::endl;
				return str;
			}
			catch(pqxx::pqxx_exception &e){
				log_messages->addField(LogFields::pqxx_failure,true);
				log_messages->addField(LogFields::pqxx_failure_string, std::string(e.base().what()));
			}
			return log_messages->single();
		}
	};

	using pool_fun_t = std::string (*) (std::shared_ptr<Remember>, int, unsigned long long);
	
        //assert(Profiler::pausedOrInactive());
	pool_fun_t pool_fun = PoolFunStruct::pool_fun;

	vector<pool_fun_t> vec{{pool_fun,pool_fun}};
	
	PreparedTest<unsigned long long> launcher{
		num_processes,vec};
	
	const auto start = high_resolution_clock::now();

	bool (*stop) (decltype(start)&) =
		[](decltype(start)& start){
		return (high_resolution_clock::now() - start) >= 15s;
	};

	std::cout << "beginning subtask generation loop" << std::endl;

	pair<int,unsigned long long> (*choose_action) (decltype(start)&)= [](auto&){
		return pair<int, unsigned long long>{0,micros(elapsed_time())};
	};

	milliseconds (*delay) (decltype(start)&) = [](auto&){
		return getArrivalInterval(actual_arrival_rate);
	};

	const std::string results = launcher.run_tests(start,stop,choose_action,delay);
		
	//resume profiling
	
	global_log->addField(GlobalsFields::final_completion_time,
						 duration_cast<milliseconds>(elapsed_time()).count());

	logFile << logger->declarations() << endl;

	logFile << global_log->single() << endl;
	logFile << results;
	

}
