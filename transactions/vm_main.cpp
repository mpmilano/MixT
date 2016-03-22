#include "ProcessPool.hpp"
#include "ThreadPool.hpp"
#include <iostream>
#include <fstream>
#include <thread>
#include <pqxx/pqxx>
#include "SQLStore.hpp"
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

constexpr int num_processes = 500;
static_assert(num_processes <= 500,"Error: you are at risk of too many open files");
constexpr auto arrival_rate = 10_Hz;

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
	double T = thousand * log(U) / arrival_rate.hertz;
	unsigned long l = round(T);
	return milliseconds(l);
}


int get_name(double alpha){
	const int max = 478446;
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
const int mod_constant = 15;

int get_strong_ip() {
	static int ip_addr{[](){
			std::string static_addr {STRONG_REMOTE_IP};
			if (static_addr.length() == 0) static_addr = "127.0.0.1";
			std::cout << static_addr << std::endl;
			return mutils::decode_ip(static_addr);
		}()};
	return ip_addr;
}

int main(){
	int ip = get_strong_ip();	
	logFile.open(log_name);
	logFile << "Begin Log for " << log_name << std::endl;
	std::cout << "hello world from VM "<< my_unique_id << " in group " << CAUSAL_GROUP << std::endl;
	std::cout << "connecting to " << string_of_ip(ip) << std::endl;
	AtScopeEnd ase{[&](){logFile << "End" << std::endl;
			logFile.close();}};
	discard(ase);
	
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

	struct Remember {
		tracker::Tracker trk;
		SQLStore<Level::strong>::SQLInstanceManager ss;
		SQLStore<Level::causal>::SQLInstanceManager sc;
		DeserializationManager dsm;
		
		Remember(int id):trk(id + 1024, tracker::CacheBehaviors::full),ss(trk),sc(trk),dsm({&ss,&sc}){}
	};
	
	std::function<std::string (std::unique_ptr<Remember>&, int, unsigned long long)> pool_fun =
		[ip](std::unique_ptr<Remember>& mem, int _pid, unsigned long long _start_time){
		auto pid = _pid % (65535 - 1025); //make sure this can be used as a port number
		
		if (!mem) {
			mem.reset(new Remember(pid));
		}
		assert(mem);
		microseconds start_time(_start_time);
		//std::cout << "launching task on pid " << pid << std::endl;
		//AtScopeEnd em{[pid](){std::cout << "finishing task on pid " << pid << std::endl;}};
		std::stringstream log_messages;
		try{
			std::string str = [&](){
				SQLStore<Level::strong> &strong = mem->ss.inst_strong(ip);
				SQLStore<Level::causal> &causal = mem->sc.inst_causal(0);
				auto &trk = mem->trk;
				assert(!strong.in_transaction());
				assert(!causal.in_transaction());
				assert(!trk.get_StrongStore().in_transaction());
				//I'm assuming that pid won't get larger than the number of allowable ports...
				assert(pid + 1024 < 49151);
				
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
								)
							auto end = high_resolution_clock::now() - launch_clock;
							log_messages << "duration: " <<
							duration_cast<milliseconds>(end - start_time).count()
							<< ((name % mod_constant) == 0 ? "ms read/write" : "ms read") << std::endl;
							break;
						}
						catch(const SerializationFailure &r){
							log_messages << "serialization failure: "
							<< duration_cast<milliseconds>(high_resolution_clock::now()
														   - launch_clock - start_time).count()
							<< std::endl;
							continue;
						}
					}
					return log_messages.str();
				};
				if (better_rand() > .7 || !causal_enabled){
					return test_fun(strong.template existingObject<HandleAccess::all,int>(trk, nullptr,name));
				}
				else return test_fun(causal.template existingObject<HandleAccess::all,int>(trk, nullptr,name));
			}();
			//std::cout << str << std::endl;
			return str;
		}
		catch(pqxx::pqxx_exception &e){
			log_messages << "pqxx failure: " << e.base().what() << std::endl;
		}
		std::cout << log_messages.str() << std::endl;
		return log_messages.str();
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
	vector<decltype(pool_fun)> pool_v {{pool_fun}};
        std::unique_ptr<ThreadPool<Remember,std::string, unsigned long long> >
                powner(new ThreadPool<Remember,std::string, unsigned long long>(pool_v,num_processes,exn_handler));
	auto &p = *powner;
	auto start = high_resolution_clock::now();

	auto bound = [&](){return (high_resolution_clock::now() - start) < 15s;};

	//log printer
	using future_list = std::list<std::future<std::unique_ptr<std::string> > >;
	std::unique_ptr<future_list> futures{new future_list()};

	auto elapsed_time = [](){return duration_cast<microseconds>(high_resolution_clock::now()
																- launch_clock).count();};
	
	std::function<decltype(p.launch(0,elapsed_time())) ()>
		launch1 {[&](){return p.launch(0,elapsed_time());}};

	std::unique_ptr<Remember> launch2_mem;
	decltype(launch1) launch2 {[&]() -> decltype(p.launch(0,elapsed_time())){
                        auto str = pool_fun(launch2_mem,400,elapsed_time());
						return std::async(std::launch::deferred,[str](){return heap_copy(str);});}};
        bool launch_with_threading = true;
	if (!launch_with_threading) std::cout << "Warning: threading disabled!" << std::endl;
	auto launch  = ( launch_with_threading ? launch1 : launch2);
	
	std::cout << "beginning subtask generation loop" << std::endl;
	while (bound()){
		std::this_thread::sleep_for(getArrivalInterval(arrival_rate * as_hertz(1 + int{concurrencySetting})));
		futures->emplace_back(launch());
	}
	
	//print everything
	while (!futures->empty()){
		std::unique_ptr<future_list> new_futures{new future_list()};
		for (auto &f : *futures){
			if (f.valid()){
				if (f.wait_for(1ms) != future_status::timeout){
					auto strp = f.get();
					if (strp){
						logFile << *strp;
					}
				}
				else {
					new_futures->push_back(std::move(f));
				}
			}
		}
		futures = std::move(new_futures);
	}
	std::cout << "all tasks done" << std::endl;
}
