#pragma once
#include <list>
#include <memory>
#include <vector>
#include <queue>
#include <cassert>
#include <iostream>
#include <map>
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

		void syncClient(DataStore& to) const {
			DataStore const &from = *this;
			for (auto& ptr_p : to.hndls) {
				auto &ptr = ptr_p.second;
				auto const &m_ptr = from.hndls.at(ptr->id);
				if (m_ptr->rid == ptr->rid) {
					ptr->grab_obj(*m_ptr);
				}
			}
		}

//hiding implemntation details here.  
#include "Backend-impl.h"

		//constructors and destructor
		DataStore () {}

		DataStore(DataStore&& ds):
			hndls(std::move(ds.hndls)){}

		DataStore (const DataStore &) = delete;
		
		friend class HandlePrime;
		template<Client_Id cid>
		friend class Client;
		
		template<Level L, typename R, typename IR>
		friend class tester::Fuzz;
	};

}

//template magic in here.
#include "handle_utils"

