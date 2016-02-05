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
#include "CooperativeCache.hpp"
#include "Basics.hpp"
#include <sys/types.h>
#include <chrono>
#include <cmath>
#include <unistd.h>
#include "ProcessPool.hpp"//*/
#include "Operate_macros.hpp"
#include "Transaction_macros.hpp"

using namespace std;
using namespace chrono;
using namespace mutils;
using namespace myria;
using namespace tracker;
using namespace mtl;
using namespace pgsql;

//test file for working with the cooperative cache.

using Ret = pair<bool,string>;
using Param = int;
using ExtraParam1 = std::function<void* (void*) >;
using ExtraParam2 = int;

Ret firstFunction(ExtraParam1, ExtraParam2, Param){
	CooperativeCache cc;
	std::map<Name,std::pair<Tracker::Clock, std::vector<char> > > testMap;
	testMap.emplace(2,pair<Tracker::Clock, vector<char> >{{{1,1,1,1}},{'c','a','f','e',0}});
	cc.listen_on(5000);
	cc.insert(1,testMap);
	return make_pair(true,"");
}

Ret secondFunction(ExtraParam1, ExtraParam2, Param){
	CooperativeCache cc;
	future<std::vector<Tracker::StampedObject> > res = cc.get({1,mutils::decode_ip("127.0.0.1")},5000);
	auto obj = res.get();
	std::cout << "we received: " << obj << std::endl;
	assert((obj.front() == Tracker::StampedObject{2,{{1,1,1,1}},{'c','a','f','e',0}}));
	cc.get({12,mutils::decode_ip("127.0.0.1")},5000).get();
	return make_pair(true,"");
}

int main(){

	std::vector<std::function<Ret (ExtraParam1, ExtraParam2, Param)> > functions;
	functions.emplace_back(firstFunction);
	functions.emplace_back(secondFunction);
	std::function<Ret (std::exception_ptr) > on_exn =
		[](std::exception_ptr exn) -> Ret {
		try {
			assert(exn);
			std::rethrow_exception(exn);
		}
		catch (const exception& e){
			return make_pair(false, e.what());
		}
		catch (...){
			return make_pair(false,"exception occurred!");
		}
	};

	Ret sfres;

	{
		//ProcessPool<Ret,Param> pool(functions,1,on_exn);

		//auto firstFuture = pool.launch(0,0);
		
		auto _ffres = firstFunction(ExtraParam1{}, ExtraParam2{}, Param{});
		auto ffres = &_ffres;
		try{
			sfres = secondFunction(ExtraParam1{}, ExtraParam2{}, Param{});
		}
		catch(const exception &e){
			cerr << e.what() << endl;
		}
		
		if (ffres && (!ffres->first)){
			std::cerr << ffres->second << std::endl;
		}
		if (!sfres.first){
			std::cerr << sfres.second << std::endl;
		}
		assert(ffres && ffres->first);
	}
	assert(sfres.first);
}
