#pragma once
#include "Backend.hpp"
#include "Pending.hpp"
#include <execinfo.h>


namespace backend {
	
	template<Client_Id cid>
	class Client {


		//BEGIN INTERNALS
	private:

		DataStore& master;
		DataStore local;
		
		bool sync_enabled = true;

		pending pending_updates;

		template<Level L, typename T, Level _L, HandleAccess _ha, Client_Id _cid>
		DataStore::Handle<cid,L,HandleAccess::all, T> 
		gethandle_internal(const DataStore::Handle<_cid,_L,_ha,T> &);
		
		template<Level L, typename T>
		DataStore::Handle<cid,L,HandleAccess::all, T> newHandle_internal(std::unique_ptr<T> );

		template<Level L, typename T>
		std::unique_ptr<T> del_internal(DataStore::Handle<cid, L,HandleAccess::all,T>& );
		//END INTERNALS

	public:
		
		Client(DataStore& master);

		Client(Client&& old);
		
		//create/delete object slots
#include "ClientCreate.hpp"

		
		//KVstore-style interface
#include "ClientKV.hpp"
		
		//commutative operations
#include "ClientCommute.hpp"
		
		friend class transaction_cls;
		//transactions interface
		class transaction_cls;
#include "Transactions.hpp"

		transaction_cls transaction(){ return transaction_cls(*this); }
		
		template<Level l>
		void waitForSync();

		template<Client_Id>
		friend class Client;
	};
}

#include "Transactions.cpp"
#include "Client.cpp"
