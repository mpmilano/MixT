
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
#include "launch_test.hpp"
//*/
#include "Transaction_macros.hpp"

using namespace std;
using namespace chrono;
using namespace mutils;
using namespace myria;
using namespace mtl;
using namespace pgsql;
using namespace tracker;


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
const int ip = get_strong_ip();

using fake_time = unsigned long long;

namespace synth_test {

	template<typename Hndl>
	using oper_f = void (*) (unique_ptr<VMObjectLog>& ,
							 Tracker &, Hndl );

	auto log_start(std::shared_ptr<Remember> mem, unique_ptr<VMObjectLog>& log_messages, fake_time _start_time){
		assert(mem);
		log_messages = mem->log_builder->template beginStruct<LoggedStructs::log>();
		microseconds start_time(_start_time);
		auto run_time = elapsed_time();
		
		log_messages->addField(LogFields::submit_time,
							   duration_cast<milliseconds>(start_time).count());
		log_messages->addField(LogFields::run_time,
							   duration_cast<milliseconds>(run_time).count());
		return start_time;
	}

	template<typename Strong, typename Causal>
	void store_asserts(Strong &strong, Causal& causal, Tracker &trk){
		assert(!strong.in_transaction());
		assert(!causal.in_transaction());
		assert(!trk.get_StrongStore().in_transaction());
	}
	
	template<typename Hndl>
	void perform_increment(unique_ptr<VMObjectLog>& log_messages,
										 Tracker &trk, Hndl hndl){
		TRANSACTION(log_messages,trk,hndl,
					do_op<RegisteredOperations::Increment>(hndl)
			)//*/
			log_messages->addField(
				LogFields::is_write,true);
	}

	template<typename Hndl>
	void perform_read(unique_ptr<VMObjectLog>& log_messages,
							 Tracker &trk, Hndl hndl){
		TRANSACTION(log_messages,
					trk,hndl,
					let_remote(tmp) = hndl IN(mtl_ignore($(tmp)))
			);
		log_messages->addField(
			LogFields::is_read,true);
		
	}

	template<typename Hndl>
	void perform_operation(unique_ptr<VMObjectLog>& log_messages,
								  Tracker &trk, Hndl hndl, oper_f<Hndl> oper){
		try{ 
			for(int tmp2 = 0; tmp2 < 10; ++tmp2){
				try{
					oper(log_messages,trk,hndl);
					auto end = elapsed_time();
					log_messages->addField(LogFields::done_time,
										   duration_cast<milliseconds>(end).count());
					break;
				}
				catch(const SerializationFailure& sf){
					auto end = elapsed_time();
					log_messages->addField(LogFields::done_time,
										   duration_cast<milliseconds>(end).count());
					log_messages->addField(LogFields::is_serialization_error,true);
					continue;
				}
			}
		}
		catch(pqxx::pqxx_exception &e){
			log_messages->addField(LogFields::pqxx_failure,true);
			log_messages->addField(LogFields::pqxx_failure_string, std::string(e.base().what()));
		}
	}

	std::string perform_strong_increment(std::shared_ptr<Remember> mem, int i,
										 fake_time _start_time){
		auto name = get_name_write();
		std::unique_ptr<VMObjectLog> log_messages;
		log_start(mem,log_messages,_start_time);
		SQLStore<Level::strong> &strong = mem->ss.inst_strong(ip);
		SQLStore<Level::causal> &causal = mem->sc.inst_causal(0);
		auto &trk = mem->trk;
		store_asserts(strong,causal,trk);
		perform_operation(log_messages, trk,
						  strong.template existingObject<HandleAccess::all,int>(log_messages,name),
						  perform_increment
			);
		log_messages->addField(LogFields::is_strong,true);
		return log_messages->single();
	}

	std::string perform_causal_increment(std::shared_ptr<Remember> mem, int i,
										 fake_time _start_time){
		auto name = get_name_write();
		std::unique_ptr<VMObjectLog> log_messages;
		log_start(mem,log_messages,_start_time);
		SQLStore<Level::strong> &strong = mem->ss.inst_strong(ip);
		SQLStore<Level::causal> &causal = mem->sc.inst_causal(0);
		auto &trk = mem->trk;
		store_asserts(strong,causal,trk);
		perform_operation(log_messages,trk,
						  causal.template existingObject<HandleAccess::all,int>(log_messages,name),
						  perform_increment
			);
		log_messages->addField(LogFields::is_causal,true);
		return log_messages->single();
		
	}

	std::string perform_strong_read(std::shared_ptr<Remember> mem, int i,
										 fake_time _start_time){
		auto name = get_name_read(0.5);
		std::unique_ptr<VMObjectLog> log_messages;
		log_start(mem,log_messages,_start_time);
		SQLStore<Level::strong> &strong = mem->ss.inst_strong(ip);
		SQLStore<Level::causal> &causal = mem->sc.inst_causal(0);
		auto &trk = mem->trk;
		store_asserts(strong,causal,trk);
		perform_operation(log_messages, trk,
						  strong.template existingObject<HandleAccess::all,int>(log_messages,name),
						  perform_read
			);
		log_messages->addField(LogFields::is_strong,true);
		return log_messages->single();
	}

	std::string perform_causal_read(std::shared_ptr<Remember> mem, int i,
										 fake_time _start_time){
		auto name = get_name_read(0.5);
		std::unique_ptr<VMObjectLog> log_messages;
		log_start(mem,log_messages,_start_time);
		SQLStore<Level::strong> &strong = mem->ss.inst_strong(ip);
		SQLStore<Level::causal> &causal = mem->sc.inst_causal(0);
		auto &trk = mem->trk;
		store_asserts(strong,causal,trk);
		perform_operation(log_messages,trk,
						  causal.template existingObject<HandleAccess::all,int>(log_messages,name),
						  perform_read
			);
		log_messages->addField(LogFields::is_causal,true);
		return log_messages->single();

	}

	template<typename T>
	pair<int,fake_time> choose_action(T&){
		bool do_write = (int_rand() % mod_constant) == 0;
		bool is_strong = better_rand() > .7 || !causal_enabled;
		if (do_write && is_strong) return pair<int,fake_time>(0,micros(elapsed_time()));
		if (do_write && !is_strong) return pair<int,fake_time>(1,micros(elapsed_time()));
		if (!do_write && is_strong) return pair<int,fake_time>(2,micros(elapsed_time()));
		if (!do_write && !is_strong) return pair<int,fake_time>(3,micros(elapsed_time()));
		assert(false);
	}
	

}

int main(){

	std::cout << "In configuration; " << (causal_enabled ? "with causal" : " with only strong" ) << std::endl;
	ofstream logFile;
	logFile.open(log_name);
	//auto prof = VMProfiler::startProfiling();
	//auto longpause = prof->pause();
	auto logger = build_VMObjectLogger();
	auto global_log = logger->template beginStruct<LoggedStructs::globals>();
	global_log->addField(GlobalsFields::request_frequency,actual_arrival_rate);
	std::cout << "hello world from VM "<< my_unique_id << " in group " << CAUSAL_GROUP << std::endl;
	std::cout << "connecting to " << string_of_ip(ip) << std::endl;

	using pool_fun_t = std::string (*) (std::shared_ptr<Remember>, int, fake_time);
	
	vector<pool_fun_t> vec{{
			synth_test::perform_strong_increment,
				synth_test::perform_causal_increment,
				synth_test::perform_strong_read,
				synth_test::perform_causal_read
				}};
	
	PreparedTest<Remember,fake_time> launcher{
		num_processes,vec};
	
	const auto start = high_resolution_clock::now();

	bool (*stop) (decltype(start)&) =
		[](decltype(start)& start){
		return (high_resolution_clock::now() - start) >= 15s;
	};

	std::cout << "beginning subtask generation loop" << std::endl;

	milliseconds (*delay) (decltype(start)&) = [](auto&){
		return getArrivalInterval(actual_arrival_rate);
	};

	const std::string results = launcher.run_tests(start,stop,synth_test::choose_action,delay);
		
	//resume profiling
	
	global_log->addField(GlobalsFields::final_completion_time,
						 duration_cast<milliseconds>(elapsed_time()).count());

	logFile << logger->declarations() << endl;

	logFile << global_log->single() << endl;
	logFile << results;
	

}
