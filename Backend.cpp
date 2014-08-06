#pragma once
#include "Backend.hpp"

namespace backend {
	
	template<Client_Id id, Level L, HandleAccess HA, typename T>	
	DataStore::Handle<id,L,HA,T> DataStore::newhandle_internal(std::unique_ptr<T> r) {
		return Handle<id,L,HA,T>
			(HandleImpl<T>::constructAndPlace(*this,std::move(r)));
	}
	
	
	template<Client_Id id, Level L, HandleAccess HA, typename T>
	std::unique_ptr<T> DataStore::del_internal(Handle<id,L,HA,T> &hndl_i){
		auto &hndl = hndl_i.hi(); 
		std::unique_ptr<T> ret = hndl;
		assert(hndls[hndl.id]->id == hndl.id);
		hndls[hndl.id].first.reset(nullptr);
		return ret;
	}

	
	template<Client_Id cid, Level L, typename T, Level _L, Client_Id _cid>
	DataStore::Handle<cid,L,HandleAccess::all, T> 
	DataStore::get_handle(const DataStore::Handle<_cid,_L,HandleAccess::all,T> &_underlying){
		const auto &underlying = _underlying.hi();
		auto &local = *this;
		auto &master = underlying.parent;
		assert(&local != &master);
		assert(local.hndls[underlying.id].get() == nullptr );
		assert(master.hndls[underlying.id].get() != nullptr );
		assert(master.hndls[underlying.id]->rid == underlying.rid);
		auto &&ret = DataStore::Handle<cid,L,HandleAccess::all,T>(underlying.clone(local));
		return std::move(ret);
	}


	void DataStore::syncClient(DataStore& to) const {
		ReadLock(const_cast<DataStore*>(this)->mut);
		WriteLock(to.mut);
		DataStore const &from = *this;
		for (auto& ptr_p : to.hndls) {
			auto &ptr = ptr_p.second;
			auto const &m_ptr = from.hndls.at(ptr->id);
			if (m_ptr->rid == ptr->rid) {
				ptr->grab_obj(*m_ptr);
			}
		}
	}

}

