#pragma once
		
		template<Level L, typename T>
		DataStore::Handle<cid, L,HandleAccess::all, T>
		newHandle(std::unique_ptr<T> r)
			{return newHandle_internal<L>(std::move(r));}
		
		template<Level L, typename T>
		DataStore::Handle<cid, L,HandleAccess::all, T> newHandle(T r)
			{return newHandle_internal<L>(std::unique_ptr<T>(new T(r)));}
		
		template<Level L, typename T>
		DataStore::Handle<cid, L,HandleAccess::all, T> newHandle(T* r = nullptr)
			{return newHandle_internal<L>(std::unique_ptr<T>(r));}
		
		template<Level L, typename T>
		std::unique_ptr<T> del(DataStore::Handle<cid, L,HandleAccess::all,T>& hndl){
			waitForSync<L>();
			return del_internal(hndl);}
		
		template<Level L, typename T>
		auto ro_hndl(DataStore::Handle<cid, L,HandleAccess::all,T> &old){
			return DataStore::Handle<cid, L,HandleAccess::read,T>(old.hi());
		}
		
		template<Level L, typename T>
		auto wo_hndl(DataStore::Handle<cid, L,HandleAccess::all,T> &old){
			return DataStore::Handle<cid, L,HandleAccess::write,T>(old.hi());
		}	

		template<Level Lnew, Level Lold, typename T>
		auto newConsistency (DataStore::Handle<cid, Lold,HandleAccess::all,T> &old) {
			return DataStore::Handle<cid, Lnew,
						 (Lold == Level::strong ? 
						  HandleAccess::read : 
						  HandleAccess::write),
						 T> (old.hi());
		}

		template<Client_Id cid_old, Level l, HandleAccess ha, typename T>
		DataStore::Handle<cid,l,ha,T> get_access(const DataStore::Handle<cid_old,l,ha,T> &hndl, 
							 const Client<cid_old> &o) {
			//static_assert(cid != cid_old, "You already have access to this handle.");
			assert (&master == &o.master);
			return gethandle_internal<l>(hndl);
		}
