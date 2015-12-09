#include "ProcessPool.hpp"
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
#include <unistd.h>//*/
#include "ProcessPool.hpp"
#include "Operate_macros.hpp"

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

const auto log_name = [](){
	auto pid = getpid();
	return std::string("/tmp/MyriaStore-output-") + std::to_string(pid);
}();
ofstream logFile;

//I'm guessing miliseconds.  Here's hoping!
long getArrivalInterval(double arrival_rate) {
	// exponential
	constexpr double thousand = -1000.0;
	double U = better_rand();
	double T = thousand * log(U) / arrival_rate;
	return round(T);
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
	printf("connecting to %d.%d.%d.%d\n",((char*)&ip)[0],((char*)&ip)[1],((char*)&ip)[2],((char*)&ip)[3]);
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

	std::function<std::string (unsigned long long)> pool_fun =
		[ip](unsigned long long start_time){
		std::stringstream log_messages;
		try{

			SQLStore<Level::strong> &strong = SQLStore<Level::strong>::inst(ip);
			SQLStore<Level::causal> &causal = SQLStore<Level::causal>::inst(0);

			auto name = get_name(0.5);
			
			auto test_fun = [&](const auto &hndl){

				for(int tmp2 = 0; tmp2 < 10; ++tmp2){
					try{
						if ((name % mod_constant) == 0)
							TRANSACTION(
								do_op(Increment,hndl)
								)
							else hndl.get();
						auto end = high_resolution_clock::now() - launch_clock;
						log_messages << "duration: " << duration_cast<microseconds>(end).count() - start_time
						<< ((name % mod_constant) == 0 ? " read/write" : " read") << std::endl;
						break;
					}
					catch(const Transaction::SerializationFailure &r){
						log_messages << "serialization failure: "
						<< duration_cast<microseconds>(high_resolution_clock::now() - launch_clock).count() - start_time
						<< std::endl;
						continue;
					}
				}
				return log_messages.str();
			};
			if (better_rand() > .7 || !causal_enabled){
				return test_fun(strong.template existingObject<HandleAccess::all,int>(name));
			}
			else return test_fun(causal.template existingObject<HandleAccess::all,int>(name));
		}
		catch(pqxx::pqxx_exception &e){
			log_messages << "pqxx failure: " << e.base().what() << std::endl;
		}
		return log_messages.str();
	};
	vector<decltype(pool_fun)> pool_v {{pool_fun}};
	std::unique_ptr<ProcessPool<std::string, unsigned long long> > powner(new ProcessPool<std::string, unsigned long long>(pool_v,1));
	auto &p = *powner;
	auto start = high_resolution_clock::now();

	auto launch = [&](){return p.launch(0,duration_cast<microseconds>(high_resolution_clock::now() - launch_clock).count());};

	
	auto bound = [&](){return duration_cast<seconds>(high_resolution_clock::now() - start).count() < 120;};

	//log printer
	using future_list = std::list<std::future<std::unique_ptr<std::string> > >;
	std::unique_ptr<future_list> futures{new future_list()};
	std::thread printer{[&](){
			while (bound()){
				for (auto &f : *futures){
					if (f.valid()){
						auto strp = f.get();
						if (strp){
							logFile << *strp;
						}
					}
				}
				std::this_thread::sleep_for(1s);
			}
		}};
	while (bound()){
		futures->emplace_back(launch());
		//std::cout << pool_fun(duration_cast<microseconds>(high_resolution_clock::now() - launch_clock).count()) << std::endl;
		std::this_thread::sleep_for(milliseconds(getArrivalInterval(20)));
		/*
		  { //clean up the log of messages
		  decltype(futures) nf{new future_list()};
		  for (auto &f : *futures){
		  if (f.valid()){
		  nf->push_back(f);
		  }
		  }
		  futures.reset(nf.release());
		  }//*/
	}
	printer.join();
}
	
