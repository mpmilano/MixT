#include "SQLStore.hpp"
#include "Transaction.hpp"
#include "Ostreams.hpp"
#include "FinalHeader.hpp"
#include "FinalizedOps.hpp"
#include <list>


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
	SQLStore<Level::strong> &strong = SQLStore<Level::strong>::inst(0);
	SQLStore<Level::causal> &causal = SQLStore<Level::causal>::inst(0);

	switch(TEST_MODE){
	case 1 : heavy_lin(strong,causal); break;
	case 2 : heavy_causal(strong,causal); break;
	case 3 : mostly_reading(strong,causal); break;
	}
}
