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
#include <unistd.h>//*/
#include "ProcessPool.hpp"
#include "Operate_macros.hpp"

constexpr int my_unique_id = IP_QUAD;

using namespace std;
using namespace chrono;

#ifdef NO_USE_STRONG
constexpr bool causal_enabled = true;
#endif
#ifdef USE_STRONG
constexpr bool causal_enabled = false;
#endif

const auto log = [](){
	auto pid = getpid();
	return std::string("/tmp/MyriaStore-output-") + std::to_string(pid);
}();
ofstream logFile;

double better_rand(){
	return ((double)rand()) / RAND_MAX;
}

long getArrivalInterval(double arrival_rate) {
	 // exponential
	double U = better_rand();
	double T = -1000.0d * log(U) / arrival_rate;
	return round(T);
 }


int get_name(double alpha){
	const int max = 220000;
	double y = better_rand();
	double max_pow = pow(max,1 - alpha);
	double x = pow(max_pow*y,1/(1-alpha));
	return round(x);
}

const auto launch_clock = high_resolution_clock::now();
const int modulus = 15;

void main(){
	int ip = 0;
	{
		char *iparr = (char*)&ip;
		//128.84.217.31
		iparr[0] = 128;
		iparr[1] = 84;
		iparr[2] = 217;
		iparr[3] = 31;
	}
	logFile << "Begin Log for " << log << std::endl;
	std::cout << "hello world from VM "<< my_unique_id << " in group " << CAUSAL_GROUP << std::endl;
	AtScopeEnd ase{[&](){logFile << "End" << std::endl;
			logFile.close();}};
	discard(ase);
	
	ProcessPool &p = *(new ProcessPool<std::string, unsigned long long>
					   ({[ip](unsigned long long start_time){
							   SQLStore<Level::strong> &strong = SQLStore<Level::strong>::inst(ip);
							   SQLStore<Level::causal> &causal = SQLStore<Level::causal>::inst(0);
							   auto name = get_name(1.5);
							   std::stringstream log_messages;
							   test_fun = [&](auto &hndl){
							   for(int tmp2 = 0; tmp2 < 10; ++tmp2){
								   try{
									   if ((name % modulus) == 0)
										   TRANSACTION(
											   do_op(Increment,hndl)
											   )
										   else hndl.get();
									   auto end = high_resolution_clock::now() - launch_clock;
									   log_messages << "duration: " << duration_cast<microseconds>(end).count() - start_time
											   << ((name % modulus) == 0 ? " read/write" : " read") << std::endl;
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
								   test_fun(strong.template existingObject<HandleAccess::all,int>(name));
							   }
							   else test_fun(causal.template existingObject<HandleAccess::all,int>(name));
						   })));
	auto start = high_resolution_clock::now();
	auto elapsed = [&](){
		return high_resolution_clock::now() - start;
	};
	auto launch = [&](){return p.launch(0,duration_cast<microseconds>(high_resolution_clock::now() - launch_clock).count());};

	
	auto bound = [](){return duration_cast<seconds>(high_resolution_clock::now() - start)::count() < 120;};

	//log printer
	using future_list = std::list<std::future<std::unique_ptr<std::string> > >;
	std::unique_ptr<future_list> futures{new future_list()};
	std::thread printer{[&](){
			const auto &cfutures = *futures;
			while (bound()){
				for (auto &f : cfutures){
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
		std::this_thread::sleep_for(getArrivalInterval());
		
		{ //clean up the log of messages
			decltype(futures) nf{new future_list()};
			for (auto &f : *futures){
				if (f.valid()){
					nf->push_back(f);
				}
			}
			futures.reset(nf.release());
		}
	}
	printer.join();
}
	
