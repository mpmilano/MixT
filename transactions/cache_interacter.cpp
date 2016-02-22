#include "FreeExpr.hpp"
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
#include "TrackerTestingStore.hpp"
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
using namespace testing;

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
	while (true) try{
			future<std::vector<Tracker::StampedObject> > res = cc.get({1,mutils::decode_ip("127.0.0.1")},5000);
			auto obj = res.get();
			std::cout << "we received: " << obj << std::endl;
			assert((obj.front() == Tracker::StampedObject{2,{{1,1,1,1}},{'c','a','f','e',0}}));
			cc.get({12,mutils::decode_ip("127.0.0.1")},5000).get();
			return make_pair(true,"");
			break;
		}
		catch (const CooperativeCache::ProtocolException& e){
			std::cout << e.what() << " (protocol exception, trying again...)" << std::endl;
		}
}

int main(){

	init_rand(5);

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

	{
		Tracker t_here{5003,5004};
		//Tracker t_there{5004};
		TrackerTestingStore<TrackerTestingMode::manual_sync, Level::strong> strong_here{t_here};
		TrackerTestingStore<TrackerTestingMode::manual_sync, Level::causal> causal_here{t_here};
		//TrackerTestingStore<Level::strong> strong_there{t_there};
		//TrackerTestingStore<Level::causal> causal_there{t_there};
		strong_here.newObject<HandleAccess::all>(t_here,nullptr,2147483659L,Tracker::Clock{{0,0,0,0}});
		{
			auto h1 = strong_here.newObject<HandleAccess::all>(t_here,nullptr,3,string("foo"));
			auto h2 = strong_here.existingObject<HandleAccess::all,string>(t_here,nullptr,3);
			auto h3 = causal_here.newObject<HandleAccess::all>(t_here,nullptr,4,string("foofoo"));
			TRANSACTION(t_here, h1, let_remote(tmp) = h1 IN(mtl_ignore($(tmp))));
			TRANSACTION(t_here, h3, let_remote(tmp) = h3 IN(mtl_ignore($(tmp))));
			TRANSACTION(t_here, h2, let_remote(tmp) = h2 IN(mtl_ignore($(tmp))));
			
			TRANSACTION(t_here, h1, let_remote(tmp1) = h1 IN( tmp1 = string("foo1") ));
			TRANSACTION(t_here, h3, let_remote(tmp1) = h3 IN( tmp1 = string("foo2") ));
			TRANSACTION(t_here, h2, let_remote(tmp1) = h2 IN( tmp1 = string("foo3") ));
		
			t_here.assert_nonempty_tracking();
			assert(h1.get(t_here,nullptr) == h2.get(t_here,nullptr));
		}

		auto& cchere = t_here.getcache();

		assert(!cchere.cache->empty());
		for (auto& e : *cchere.cache){
			std::cout << e.first << " --> " << e.second << std::endl;
		}
		
		CooperativeCache cc;
		while (true) try{
				future<std::vector<Tracker::StampedObject> > res = cc.get({585950151,mutils::decode_ip("127.0.0.1")},5003);
				auto obj = res.get();
				std::cout << "we received: " << obj << std::endl;
				break;
			}
			catch (const CooperativeCache::ProtocolException& e){
				std::cout << e.what() << " (protocol exception, trying again...)" << std::endl;
			}
			catch (const CooperativeCache::CacheMiss& e){
				std::cout << e.what() << std::endl;
				break;
			}

		{
			Tracker t_there{5004,5003};
			TrackerTestingStore<TrackerTestingMode::manual_sync, Level::strong> strong_there{t_there};
			TrackerTestingStore<TrackerTestingMode::manual_sync, Level::causal> causal_there{t_there};
			auto h2 = strong_there.existingObject<HandleAccess::all,string>(t_there,nullptr,3);
			auto h3 = causal_there.existingObject<HandleAccess::all,string>(t_there,nullptr,4);
			//TRANSACTION(t_there, h1, let_remote(tmp) = h1 IN(mtl_ignore($(tmp))));
			TRANSACTION(t_there, h3, let_remote(tmp1) = h3 IN( tmp1 = string("foo2") ));
			TRANSACTION(t_there, h2, let_remote(tmp1) = h2 IN( tmp1 = string("foo3") ));
			TRANSACTION(t_there, h3, let_remote(tmp) = h3 IN(mtl_ignore($(tmp))));
			TRANSACTION(t_there, h2, let_remote(tmp) = h2 IN(mtl_ignore($(tmp))));
			TRANSACTION(t_there, h3, let_remote(tmp1) = h3 IN( tmp1 = string("foo2") ));
			TRANSACTION(t_there, h2, let_remote(tmp1) = h2 IN( tmp1 = string("foo3") ));
			std::cout << "marker" << std::endl;
			TRANSACTION(t_there, h3, let_remote(tmp) = h3 IN(mtl_ignore($(tmp))));
			TRANSACTION(t_there, h2, let_remote(tmp) = h2 IN(mtl_ignore($(tmp))));

		}

	}


	assert(false && "oh boy");

	{
		Tracker t_here{5005,5006};
		Tracker t_there{5006,5005};
		TrackerTestingStore<TrackerTestingMode::manual_sync, Level::strong> strong_here{t_here};
		TrackerTestingStore<TrackerTestingMode::manual_sync, Level::causal> causal_here{t_here};
		TrackerTestingStore<TrackerTestingMode::manual_sync, Level::strong> strong_there{t_there};
		TrackerTestingStore<TrackerTestingMode::manual_sync, Level::causal> causal_there{t_there};
		strong_here.newObject<HandleAccess::all>(t_here,nullptr,2147483659L,Tracker::Clock{{0,0,0,0}});

		{
			// "here" transactions
			auto h1 = strong_here.newObject<HandleAccess::all>(t_here,nullptr,3,string("foo"));
			auto h3 = causal_here.newObject<HandleAccess::all>(t_here,nullptr,4,string("foofoo"));
			TRANSACTION(t_here, h1, let_remote(tmp) = h1 IN(mtl_ignore($(tmp))));
			TRANSACTION(t_here, h3, let_remote(tmp) = h3 IN(mtl_ignore($(tmp))));
			
			TRANSACTION(t_here, h1, let_remote(tmp1) = h1 IN( tmp1 = string("foo1") ));
			TRANSACTION(t_here, h3, let_remote(tmp1) = h3 IN( tmp1 = string("foo2") ));
		}

		{
			//"there" transactions
			auto h2 = strong_there.existingObject<HandleAccess::all,string>(t_there,nullptr,3);
			auto h3 = causal_there.existingObject<HandleAccess::all,string>(t_there,nullptr,4);
			TRANSACTION(t_there, h3, let_remote(tmp1) = h3 IN( tmp1 = string("foo2") ));
			TRANSACTION(t_there, h2, let_remote(tmp1) = h2 IN( tmp1 = string("foo3") ));
			TRANSACTION(t_there, h3, let_remote(tmp) = h3 IN(mtl_ignore($(tmp))));
			TRANSACTION(t_there, h2, let_remote(tmp) = h2 IN(mtl_ignore($(tmp))));
			TRANSACTION(t_there, h3, let_remote(tmp1) = h3 IN( tmp1 = string("foo2") ));
			TRANSACTION(t_there, h2, let_remote(tmp1) = h2 IN( tmp1 = string("foo3") ));
			TRANSACTION(t_there, h3, let_remote(tmp) = h3 IN(mtl_ignore($(tmp))));
			TRANSACTION(t_there, h2, let_remote(tmp) = h2 IN(mtl_ignore($(tmp))));
		}
		
	}
}
