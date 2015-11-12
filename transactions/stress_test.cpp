#include "SQLStore.hpp"
#include "Transaction.hpp"
#include "Ostreams.hpp"
#include "FinalHeader.hpp"
#include "FinalizedOps.hpp"
#include <list>
#include <pqxx/pqxx>

using namespace std;

const std::list<int> names_strong = {5,7,9,11,13};
const std::list<int> names_causal = {6,8,10,12,14};


void setup(SQLStore<Level::strong> &strong, SQLStore<Level::causal> &causal){
	for (auto name : names_strong){
		if (!strong.exists(name)){
			strong.template newObject<HandleAccess::all>(name,1337);
		}
	}
	
	for (auto name : names_strong){
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

void heavy_lin(SQLStore<Level::strong> &strong, SQLStore<Level::causal> &causal){
	while(true){
		for (auto name : names_strong){
			{
				auto obj = strong.template existingObject<HandleAccess::all,int>(name);
				obj.get();
			}
			increment(strong,name);
		}
	}
}

void heavy_causal(SQLStore<Level::strong> &strong, SQLStore<Level::causal> &causal){
	assert(false);
}

void mostly_reading(SQLStore<Level::strong> &strong, SQLStore<Level::causal> &causal){
	assert(false);
}

int main(){
	int ip = 0;
	char *iparr = (char*)&ip;
	//128.84.217.139
	iparr[0] = 128;
	iparr[1] = 84;
	iparr[2] = 217;
	iparr[3] = 139;
	try{
		SQLStore<Level::strong> &strong = SQLStore<Level::strong>::inst(ip);
		SQLStore<Level::causal> &causal = SQLStore<Level::causal>::inst(ip);
		
		switch(TEST_MODE){
		case 1 : heavy_lin(strong,causal); break;
		case 2 : heavy_causal(strong,causal); break;
		case 3 : mostly_reading(strong,causal); break;
		}
	}
	catch (const pqxx::pqxx_exception &r){
		std::cerr << r.base().what() << std::endl;
		assert(false && "exec failed");
	}
}
