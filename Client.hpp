#pragma once
#include "Backend.hpp"
#include "Pending.hpp"


namespace backend {
	
	template<Client_Id cid>
	class Client {


		//BEGIN INTERNALS
	private:

		DataStore& master;
		DataStore local;
		
		bool sync_enabled = true;
		bool all_final = false;

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
		template<Level L, typename T>
		DataStore::Handle<cid, L,HandleAccess::all, T>
		newHandle(std::unique_ptr<T> r);
		
		template<Level L, typename T>
		DataStore::Handle<cid, L,HandleAccess::all, T> newHandle(T r);
		
		template<Level L, typename T>
		DataStore::Handle<cid, L,HandleAccess::all, T> newHandle(T* r = nullptr);
		
		template<Level L, typename T>
		std::unique_ptr<T> del(DataStore::Handle<cid, L,HandleAccess::all,T>& hndl);
		
		template<Level L, typename T>
		auto ro_hndl(DataStore::Handle<cid, L,HandleAccess::all,T> &old);
		
		template<Level L, typename T>
		auto wo_hndl(DataStore::Handle<cid, L,HandleAccess::all,T> &old);

		template<Level Lnew, Level Lold, typename T>
		auto newConsistency (DataStore::Handle<cid, Lold,HandleAccess::all,T> &old);

		template<Client_Id cid_old, Level l, HandleAccess ha, typename T>
		DataStore::Handle<cid,l,ha,T> get_access(const DataStore::Handle<cid_old,l,ha,T> &hndl, 
							 const Client<cid_old> &o);
		
		//KVstore-style interface
		template<Level L, typename T, HandleAccess HA>
		typename std::enable_if<canRead(HA), T&>::type
		get(DataStore::Handle<cid,L, HA, T> &hndl);
		
		template<Level L, typename T, HandleAccess HA>
		typename std::enable_if<canWrite(HA), void>::type
		give(DataStore::Handle<cid, L, HA, T> &hndl, std::unique_ptr<T> obj);
		
		template<Level L, typename T, HandleAccess HA>
		typename std::enable_if<canWrite(HA), void>::type
		give(DataStore::Handle<cid, L, HA, T> &hndl, T* obj);
		
		template<Level L, typename T>
		std::unique_ptr<T> take(DataStore::Handle<cid, L,HandleAccess::all,T>& hndl);
		
		//commutative operations
		template<Level L, typename T, HandleAccess HA>
		typename std::enable_if<canWrite(HA), void>::type
		incr_op(DataStore::Handle<cid, L, HA, T> &h);
		
		template<Level L, typename T, HandleAccess HA>
		typename std::enable_if<canWrite(HA), void>::type
		incr(DataStore::Handle<cid, L, HA, T> &h);
		
		template<Level L, typename T, HandleAccess HA, typename F, typename... A>
		typename std::enable_if<canRead(HA), void>::type
		add(DataStore::Handle<cid, L, HA, T> &h, F f2, A... args);
		
		
		friend class transaction_cls;
		//transactions interface
		class transaction_cls;		
		
		transaction_cls transaction(){ return transaction_cls(*this); }
		
		template<Level l>
		void waitForSync();
		
		template<Client_Id>
		friend class Client;
	};
}

#include "Transactions.cpp"
#include "RWTransaction.cpp"
#include "Client.cpp"
#include "ClientKV.cpp"
#include "ClientCreate.cpp"
#include "ClientCommute.cpp"
