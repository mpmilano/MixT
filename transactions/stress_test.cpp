#include "SQLStore.hpp"
#include "Transaction.hpp"
#include "Ostreams.hpp"
#include "FinalHeader.hpp"
#include "FinalizedOps.hpp"
#include <vector>
#include <pqxx/pqxx>
#include <chrono>

using namespace std;
using namespace chrono;

const std::vector<int> names_strong = {5,7,9,11,13};
const std::vector<int> names_causal = {6,8,10,12,14};
const auto start = high_resolution_clock::now();

void setup(SQLStore<Level::strong> &strong, SQLStore<Level::causal> &causal){
	for (auto name : names_strong){
		if (!strong.exists(name)){
			strong.template newObject<HandleAccess::all>(name,1337);
		}
	}
	
	for (auto name : names_causal){
		if (!causal.exists(name)){
			causal.template newObject<HandleAccess::all>(name,1337);
		}
	}
}

template<typename Store>
void increment(Store &s, int i){
	auto obj = s.template existingObject<HandleAccess::all,int>(i);
	TRANSACTION(
		do_op(Increment,obj));
}

template<typename Store>
void heavy_single(Store &store, const std::vector<int>& names){
	int count = 0;
	while(true){
		for (auto name : names){
			std::cout << name << "-loop " << ++count << " time " << duration_cast<microseconds>(high_resolution_clock::now() - start).count() <<std::endl;
			{
				auto obj = store.template existingObject<HandleAccess::all,int>(name);
				obj.get();
			}
			increment(store,name);
		}
	}
}


void heavy_both(SQLStore<Level::strong> &strong, SQLStore<Level::causal> &causal){
	int count = 0;
	while(true){
		for (int i = 0; i < names_strong.size(); ++i){
			{
				int name = names_strong[i];
				std::cout << "loop " << ++count << " time " << duration_cast<microseconds>(high_resolution_clock::now() - start).count() <<std::endl;
				{
					auto obj = strong.template existingObject<HandleAccess::all,int>(name);
					obj.get();
				}
				increment(strong,name);
			}
			{
				int name = names_causal[i];
				std::cout << "loop " << ++count << " time " << duration_cast<microseconds>(high_resolution_clock::now() - start).count() <<std::endl;
				{
					auto obj = causal.template existingObject<HandleAccess::all,int>(name);
					obj.get();
				}
				increment(causal,name);
			}
			
		}
	}
}


int main(){
	int ip = 0;
	
	char *iparr = (char*)&ip;
	//128.84.217.31
	iparr[0] = 128;
	iparr[1] = 84;
	iparr[2] = 217;
	iparr[3] = 31;
	std::cout << "running in mode " << TEST_MODE << std::endl;
	try{
		SQLStore<Level::strong> &strong = SQLStore<Level::strong>::inst(ip);
		SQLStore<Level::causal> &causal = SQLStore<Level::causal>::inst(0);
		for (int i = 0; i < 1000000; ++i){
			if (!strong.exists(i)){
				strong.template newObject<HandleAccess::all>(i,1337);
			}
			if (!causal.exists(i)){
				causal.template newObject<HandleAccess::all>(i,1337);
			}
		}
		exit(0);
		setup(strong,causal);
		
		switch(TEST_MODE){
		case 1 : heavy_single(strong,names_strong); break;
		case 2 : heavy_single(causal,names_causal); break;
		case 3 : heavy_both(strong,causal); break;
		}
	}
	catch (const pqxx::pqxx_exception &r){
		std::cerr << r.base().what() << std::endl;
		assert(false && "exec failed");
	}
}
