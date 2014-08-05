#pragma once
#include <list>
#include <memory>
#include <vector>
#include <queue>
#include <cassert>
#include <iostream>
#include <map>
#include <mutex>
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

	class DataStore {

	public:

		//So that you can store lists of these things, if you want.
		class GenericHandle;

		template<typename T>
		class TypedHandle; //extends GenericHandle

		template<Client_Id cid, Level L, HandleAccess HA, typename T>
		class Handle; //extends TypedHandle<T>

	private:
		std::mutex mut;


//hiding implemntation details here.  
#include "Backend-impl.h"

		//constructors and destructor
		DataStore () {}

		DataStore(DataStore&& ds):
			hndls(std::move(ds.hndls)){}

		DataStore (const DataStore &) = delete;
		
		template<Level L, typename R, typename IR>
		friend class tester::Fuzz;

		template<Client_Id cid, Level L, typename T, Level _L, HandleAccess _ha, Client_Id _cid>
		auto get_handle(const DataStore::Handle<_cid,_L,_ha,T> &_underlying){
			mut.lock();
			const auto &underlying = _underlying.hi();
			auto &local = *this;
			auto &master = underlying.parent;
			assert(&local != &master);
			assert(local.hndls[underlying.id].get() == nullptr );
			assert(master.hndls[underlying.id].get() != nullptr );
			assert(master.hndls[underlying.id]->rid == underlying.rid);
			auto &&ret = DataStore::Handle<cid,L,HandleAccess::all,T>(underlying.clone(local));
			mut.unlock();
			return std::move(ret);
		}

		void syncClient(DataStore& to) const {
			const_cast<DataStore*>(this)->mut.lock();
			DataStore const &from = *this;
			for (auto& ptr_p : to.hndls) {
				auto &ptr = ptr_p.second;
				auto const &m_ptr = from.hndls.at(ptr->id);
				if (m_ptr->rid == ptr->rid) {
					ptr->grab_obj(*m_ptr);
				}
			}
			const_cast<DataStore*>(this)->mut.unlock();
		}


		template<Client_Id id, Level L, HandleAccess HA, typename T>	
		auto newhandle_internal(std::unique_ptr<T> r) {
			return Handle<id,L,HA,T>
				(HandleImpl<T>::constructAndPlace(*this,std::move(r)));
		}

		template<Client_Id id, Level L, HandleAccess HA, typename T>
		auto del_internal(Handle<id,L,HA,T> &hndl_i){
			auto &hndl = hndl_i.hi(); 
			std::unique_ptr<T> ret = hndl;
			assert(hndls[hndl.id]->id == hndl.id);
			hndls[hndl.id].first.reset(nullptr);
			return ret;
		}


	};

}

//template magic in here.
#include "handle_utils"

