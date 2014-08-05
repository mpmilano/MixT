	private:

		DataStore& master;
		DataStore local;
		
		bool sync_enabled = true;

		pending pending_updates;

		template<Level L, typename T, Level _L, HandleAccess _ha, Client_Id _cid>
		DataStore::Handle<cid,L,HandleAccess::all, T>
		gethandle_internal(const DataStore::Handle<_cid,_L,_ha,T> &underlying){
			return local.get_handle<cid,L>(underlying);
		}

		
		template<Level L, typename T>
		DataStore::Handle<cid,L,HandleAccess::all, T>
		newHandle_internal(std::unique_ptr<T> r){
			return gethandle_internal<L,T>(master.newhandle_internal<cid,L,HandleAccess::all>(std::move(r)));
		}

		template<Level L, typename T>
		std::unique_ptr<T> del_internal(DataStore::Handle<cid, L,HandleAccess::all,T>& hndl) {
			auto &&ret = local.del_internal<L>(hndl);
			return std::move(ret);
		}
