#include "ProcessPool.hpp"
#include "ThreadPool.hpp"
#include <iostream>
#include <fstream>
#include <thread>
#include <pqxx/pqxx>
#include "SQLStore.hpp"
#include "TrackerTestingStore.hpp"
#include "FinalHeader.hpp"
#include "FinalizedOps.hpp"
#include "Ostreams.hpp"
#include "tuple_extras.hpp"
#include "Basics.hpp"
#include <sys/types.h>
#include <chrono>
#include <cmath>
#include <unistd.h>
#include "ProcessPool.hpp"//*/
#include "Operate_macros.hpp"
#include "Transaction_macros.hpp"
#include "Hertz.hpp"
#include "ObjectBuilder.hpp"
#include "TrackerTestingStore.hpp"

constexpr int my_unique_id = int{IP_QUAD};

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

constexpr int num_processes = 100;
static_assert(num_processes <= 100,"Error: you are at risk of too many open files");
constexpr auto arrival_rate = 200_Hz;
constexpr auto actual_arrival_rate = arrival_rate * as_hertz(1 + int{concurrencySetting});

const auto log_name = [](){
	auto pid = getpid();
	return std::string("/tmp/MyriaStore-output-") + std::to_string(pid);
}();
ofstream logFile;

//I'm guessing miliseconds.  Here's hoping!
auto getArrivalInterval(Frequency arrival_rate) {
	// exponential
	constexpr double thousand = -1000.0;
	double U = better_rand();
	double T = thousand * log(U) / (arrival_rate.hertz / 10);
	unsigned long l = round(T);
	return milliseconds(l);
}

constexpr int name_max = 478446;
int get_name(double alpha){
	constexpr int max = name_max;
	double y = better_rand();
	assert (y < 1.1);
	assert (y > -0.1);
	double max_pow = pow(max,1 - alpha);
	double x = pow(max_pow*y,1/(1-alpha));
	auto ret = round(x);
	if (ret > (max + 14)) {
		std::cerr << "Name out of range! Trying again" << std::endl;
		return get_name(alpha);
	}
	else return ret + 14;
}

const auto launch_clock = high_resolution_clock::now();
const int mod_constant = 50;

int get_strong_ip() {
	static int ip_addr{[](){
			std::string static_addr {STRONG_REMOTE_IP};
			if (static_addr.length() == 0) static_addr = "127.0.0.1";
			std::cout << static_addr << std::endl;
			return mutils::decode_ip(static_addr);
		}()};
	return ip_addr;
}

template<typename A, typename B>
auto micros(duration<A,B> time){
	return duration_cast<microseconds>(time).count();
}

//was as microseconds
auto elapsed_time() {
	return high_resolution_clock::now() - launch_clock;
};

int main(){
	int ip = get_strong_ip();	
	logFile.open(log_name);
	auto logger = build_VMObjectLogger();
	auto &global_log = logger->template beginStruct<LoggedStructs::globals>();
	global_log.addField(GlobalsFields::request_frequency,actual_arrival_rate);
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
	{
		using namespace ::myria::testing;
		unique_ptr<VMObjectLogger> log_builder{build_VMObjectLogger()};
		ReassignableReference<abs_StructBuilder> current_log_builder
			{log_builder->template beginStruct<LoggedStructs::log>()};
		tracker::Tracker trk{1025,current_log_builder,tracker::CacheBehaviors::none};
		TrackerTestingStore<Level::strong> strong{trk,current_log_builder};
		TrackerTestingStore<Level::causal> causal{trk,current_log_builder};
		for (int i = 0; i <= name_max; ++i){
			if (!strong.exists(i))
				strong.template newObject<HandleAccess::all,int>(trk,nullptr,i,0);
			if (!causal.exists(i))
				causal.template newObject<HandleAccess::all,int>(trk,nullptr,i,0);
		}

		std::cout << "trackertesting init complete" << std::endl;
	}

	using namespace testing;
	
	struct Remember {
		
		unique_ptr<VMObjectLogger> log_builder{build_VMObjectLogger()};
		
		ReassignableReference<abs_StructBuilder> current_log_builder
			{log_builder->template beginStruct<LoggedStructs::log>()};
		
		tracker::Tracker trk;
		//SQLStore<Level::strong>::SQLInstanceManager ss;
		//SQLStore<Level::causal>::SQLInstanceManager sc;
		TrackerTestingStore<Level::strong> ss;
		TrackerTestingStore<Level::causal> sc;
		DeserializationManager dsm;
		
		Remember(int id)
			:trk(id + 1024, current_log_builder, tracker::CacheBehaviors::full),
			 ss(trk,current_log_builder),
			 sc(trk,current_log_builder),
			 dsm({/*&ss,&sc*/}){}
	};
	
	std::function<std::string (std::unique_ptr<Remember>&, int, unsigned long long)> pool_fun =
		[ip](std::unique_ptr<Remember>& mem, int, unsigned long long _start_time){
		assert(mem);
		AtScopeEnd ase{[&](){
				mem->current_log_builder.reset(mem->log_builder->template beginStruct<LoggedStructs::log>());
			}};
		microseconds start_time(_start_time);
		auto run_time = elapsed_time();
		
		abs_StructBuilder &log_messages = mem->current_log_builder;
		log_messages.addField(LogFields::submit_time,duration_cast<milliseconds>(start_time).count());
		log_messages.addField(LogFields::run_time,duration_cast<milliseconds>(run_time).count());
		//std::cout << "launching task on pid " << pid << std::endl;
		//AtScopeEnd em{[pid](){std::cout << "finishing task on pid " << pid << std::endl;}};
		try{
			std::string str = [&](){
				//SQLStore<Level::strong> &strong = mem->ss.inst_strong(ip);
				//SQLStore<Level::causal> &causal = mem->sc.inst_causal(0);
				auto &strong = mem->ss;
				auto &causal = mem->sc;
				auto &trk = mem->trk;
				assert(!strong.in_transaction());
				assert(!causal.in_transaction());
				assert(!trk.get_StrongStore().in_transaction());
				
				auto name = get_name(0.5);
			
				auto test_fun = [&](auto hndl){

					for(int tmp2 = 0; tmp2 < 10; ++tmp2){
						try{
							if ((name % mod_constant) == 0){
								TRANSACTION(trk,hndl,
									do_op(Increment,hndl)
									)//*/
							}
							else TRANSACTION(
								trk,hndl,
								let_remote(tmp) = hndl IN(mtl_ignore($(tmp)))
								);
							auto end = elapsed_time();
							log_messages.addField(LogFields::done_time,
												  duration_cast<milliseconds>(end).count());
							log_messages.addField(LogFields::is_write,(name % mod_constant) == 0);
							log_messages.addField(LogFields::is_serialization_error,false);
							break;
						}
						catch(const SerializationFailure &r){
							auto end = elapsed_time();
							log_messages.addField(LogFields::done_time,
												  duration_cast<milliseconds>(end).count());
							log_messages.addField(LogFields::is_serialization_error,true);
							continue;
						}
					}
					return log_messages.single();
				};
				if (better_rand() > .7 || !causal_enabled){
					return test_fun(strong.template
									existingObject<HandleAccess::all,int>(trk, nullptr,name));
				}
				else return test_fun(causal.template
									 existingObject<HandleAccess::all,int>(trk, nullptr,name));
			}();
			//std::cout << str << std::endl;
			return str;
		}
		catch(pqxx::pqxx_exception &e){
			log_messages.addField(LogFields::pqxx_failure,true);
			log_messages.addField(LogFields::pqxx_failure_string, std::string(e.base().what()));
		}
		return log_messages.single();
	};
	
	std::function<std::string (std::exception_ptr) > exn_handler = [](std::exception_ptr eptr){
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
		}
		return log_messages.str();
	};

	std::function<void (std::unique_ptr<Remember>&, int)> pool_mem_init =
		[](std::unique_ptr<Remember>& mem, int _pid){
		auto pid = _pid % (65535 - 1025); //make sure this can be used as a port number
		if (!mem) {
			mem.reset(new Remember(pid));
		}
		//I'm assuming that pid won't get larger than the number of allowable ports...
		assert(pid + 1024 < 49151);
	};
	
	vector<decltype(pool_fun)> pool_v {{pool_fun}};
        std::unique_ptr<ThreadPool<Remember,std::string, unsigned long long> >
			powner(new ThreadPool<Remember,std::string, unsigned long long>
				   (pool_mem_init,pool_v,num_processes,exn_handler));
	auto &p = *powner;
	auto start = high_resolution_clock::now();

	auto bound = [&](){return (high_resolution_clock::now() - start) < 15s;};

	//log printer
	using future_list = std::list<std::future<std::unique_ptr<std::string> > >;
	std::unique_ptr<future_list> futures{new future_list()};

	std::function<decltype(p.launch(0,micros(elapsed_time()))) ()>
		launch1 {[&](){return p.launch(0,micros(elapsed_time()));}};

	std::unique_ptr<Remember> launch2_mem;
	decltype(launch1) launch2 {[&]() -> decltype(p.launch(0,micros(elapsed_time()))){
			auto str = pool_fun(launch2_mem,400,micros(elapsed_time()));
			return std::async(std::launch::deferred,[str](){return heap_copy(str);});}};
        bool launch_with_threading = true;
	if (!launch_with_threading) std::cout << "Warning: threading disabled!" << std::endl;
	auto launch  = ( launch_with_threading ? launch1 : launch2);
	
	std::cout << "beginning subtask generation loop" << std::endl;
	while (bound()){
		std::this_thread::sleep_for(getArrivalInterval(actual_arrival_rate));
		futures->emplace_back(launch());
	}

	global_log.addField(GlobalsFields::final_completion_time,
						duration_cast<milliseconds>(elapsed_time()).count());

	logFile << logger->declarations() << endl;

	logFile << global_log.single() << endl;

	//print everything
	while (!futures->empty()){
		std::unique_ptr<future_list> new_futures{new future_list()};
		for (auto &f : *futures){
			if (f.valid()){
				if (f.wait_for(1ms) != future_status::timeout){
					auto strp = f.get();
					if (strp){
						logFile << *strp << endl;
					}
				}
				else {
					new_futures->push_back(std::move(f));
				}
			}
		}
		futures = std::move(new_futures);
	}
}
