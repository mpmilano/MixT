#include "Backend.hpp"

namespace backend{

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
}
