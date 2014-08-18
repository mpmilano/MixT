#pragma once
#include "Client.hpp"

namespace backend{	

	template<Client_Id cid>
	template<Level L, typename T, Level _L, HandleAccess _ha, Client_Id _cid>
	DataStore::Handle<cid,L,HandleAccess::all, T>
	Client<cid>::gethandle_internal(const DataStore::Handle<_cid,_L,_ha,T> &underlying){
		return local.get_handle<cid,L>(underlying);
	}
	
	template<Client_Id cid>		
	template<Level L, typename T>
	DataStore::Handle<cid,L,HandleAccess::all, T>
	Client<cid>::newHandle_internal(std::unique_ptr<T> r){
		return gethandle_internal<L,T>(master.newhandle_internal<cid,L,HandleAccess::all>(std::move(r)));
	}
	
	template<Client_Id cid>
	template<Level L, typename T>
	std::unique_ptr<T> Client<cid>::del_internal(DataStore::Handle<cid, L,HandleAccess::all,T>& hndl) {
			auto &&ret = local.del_internal<L>(hndl);
			return std::move(ret);
	}

	template<Client_Id cid>
	Client<cid>::Client(DataStore& master):master(master){
		master.syncClient(local);
	}
	
	template<Client_Id cid>
	Client<cid>::Client(Client&& old):
		master(old.master),
		local(std::move(old.local)),
		pending_updates(std::move(old.pending_updates)){}
	

	template<Client_Id cid>
	template<Level l>
	void Client<cid>::waitForSync(){
		if (!sync_enabled) return;
		if (is_strong(l) || all_final) {
			sync_enabled = false;
			master.syncClient(local);
			pending_updates.runAndClear();
			local.syncClient(master);
			assert(pending_updates.isClear());
			sync_enabled = true;
		}
	}
}
