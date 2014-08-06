#pragma once
#include <list>
#include <memory>
#include <vector>
#include <queue>
#include <cassert>
#include <iostream>
#include <map>
#include <array>
#include <mutex>
#include <shared_mutex>
#include "extras"

#define LVALUE(x) typename add_lvalue_reference<x>::type

namespace backend {
	//"strong" is Top here.  Linearizable, + start-time ordered
	//"causal" is GLUT.
	enum class Level { causal, strong};
}

namespace tester{	
	template<backend::Level L, typename R, typename IR>
	class Fuzz;
}

namespace backend{


	enum class HandleAccess {read, write, all};
	
	static constexpr bool canWrite(HandleAccess ha){
		return ha == HandleAccess::write ? true 
			: (ha == HandleAccess::all ? 
			   true : false);
	}

	static constexpr bool canRead(HandleAccess ha){
		return ha == HandleAccess::read ? true
			: (ha == HandleAccess::all ? 
			   true : false);
	}

	typedef int Client_Id;

	template<Client_Id cid>
	class Client;

	class HandlePrime;

	class DataStore {

	public:

		//So that you can store lists of these things, if you want.
		class GenericHandle;

		template<typename T>
		class TypedHandle; //extends GenericHandle

		template<Client_Id cid, Level L, HandleAccess HA, typename T>
		class Handle; //extends TypedHandle<T>

	private:
		typedef std::shared_timed_mutex Mutex;
		typedef std::unique_lock<Mutex> WriteLock;
		typedef std::shared_lock<Mutex> ReadLock;
		Mutex mut;

		template<typename T>
		class HandleImpl;
		
		std::map<uint, std::unique_ptr<HandlePrime> > hndls;
	public:

		//constructors and destructor
		DataStore () {}

		DataStore(DataStore&& ds):
			hndls(std::move(ds.hndls)){}

		DataStore (const DataStore &) = delete;
		
		template<Level L, typename R, typename IR>
		friend class tester::Fuzz;

		template<Client_Id cid, Level L, typename T, Level _L, HandleAccess _ha, Client_Id _cid>
		auto get_handle(const Handle<_cid,_L,_ha,T> &_underlying);

		void syncClient(DataStore& to) const;

		template<Client_Id id, Level L, HandleAccess HA, typename T>	
		Handle<id,L,HA,T> newhandle_internal(std::unique_ptr<T> r);

		template<Client_Id id, Level L, HandleAccess HA, typename T>
		std::unique_ptr<T> del_internal(Handle<id,L,HA,T> &hndl_i);

	};
	
	

}

//stupid templates and separate compilation
#include "HandleBackend.cpp"
#include "Handle.cpp"
#include "Backend.cpp"

//template magic in here.
#include "handle_utils"

