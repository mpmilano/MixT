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

namespace backend {

	class DataStore {

	public:
		template<Client_Id cid, Level L, HandleAccess HA, typename T>
		class Handle;
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

		template<Client_Id cid, Level L, typename T, Level _L, Client_Id _cid>
		Handle<cid,L,HandleAccess::all, T> get_handle(const Handle<_cid,_L,HandleAccess::all,T> &_underlying);

		void syncClient(DataStore& to) const;

		template<Client_Id id, Level L, HandleAccess HA, typename T>	
		Handle<id,L,HA,T> newhandle_internal(std::unique_ptr<T> r);

		template<Client_Id id, Level L, HandleAccess HA, typename T>
		std::unique_ptr<T> del_internal(Handle<id,L,HA,T> &hndl_i);
    
	};
}
