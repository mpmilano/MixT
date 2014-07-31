#pragma once
#include "Backend.hpp"
#include <execinfo.h>

namespace backend {
	
	template<Client_Id cid>
	class Client {
	private:
		DataStore& master;
		DataStore local;
		std::list<std::function<void ()> > pending_updates;

		
		template<Level L, typename T>
		DataStore::Handle<cid,L,HandleAccess::all, T>
		gethandle_internal(const DataStore::HandleImpl<T> &underlying){
			assert(local.hndls[underlying.id].get() == nullptr );
			assert(master.hndls[underlying.id].get() != nullptr );
			assert(master.hndls[underlying.id]->rid == underlying.rid );
			std::unique_ptr <DataStore::HandlePrime> &&ret = master.hndls[underlying.id]->clone(local);
			auto* ptr = dynamic_cast<DataStore::HandleImpl<T>* >(ret.get());
			assert(ptr);
			auto &h = *ptr;
			local.hndls[ret->id] = std::move(ret);
			return DataStore::Handle<cid,L,HandleAccess::all,T>(h);
		}

		
		template<Level L, typename T>
		DataStore::Handle<cid,L,HandleAccess::all, T>
		newHandle_internal(std::unique_ptr<T> r){
			return gethandle_internal<L,T>(master.newhandle_internal<cid,L,HandleAccess::all>(std::move(r)).hi());
		}

		template<Level L, typename T>
		std::unique_ptr<T> del_internal(DataStore::Handle<cid, L,HandleAccess::all,T>& hndl) {
			auto &&ret = local.del_internal<L>(hndl);
			return std::move(ret);
		}



	public:
		
		Client(DataStore& master):master(master){
			master.syncClient(local);
		}

		Client(Client&& old):
			master(old.master),
			local(std::move(old.local)),
			pending_updates(std::move(old.pending_updates)){}
		
		//create/delete object slots
		
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
		std::unique_ptr<T> del(DataStore::Handle<cid, L,HandleAccess::all,T>& hndl)
			{return del_internal(hndl);}
		
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
		DataStore::Handle<cid,l,ha,T> get_access(DataStore::Handle<cid_old,l,ha,T> &hndl, Client<cid_old> &o) {
			static_assert(cid != cid_old, "You already have access to this handle.");
			assert (&master == &o.master);
			return gethandle_internal<l>((DataStore::HandleImpl<T>&) *master.hndls[hndl.hi().id]);
		}

		
		//KVstore-style interface
		
		template<typename T, HandleAccess HA>
		typename std::enable_if<canRead(HA), T&>::type
		get(DataStore::Handle<cid,Level::causal, HA, T> &hndl)
			{return hndl.hi();}

		template<typename T, HandleAccess HA>
		typename std::enable_if<canRead(HA), T&>::type
		get(DataStore::Handle<cid,Level::strong, HA, T> &hndl) {
			waitForSync(); return hndl.hi();
		}

		
		template<Level L, typename T, HandleAccess HA>
		typename std::enable_if<canWrite(HA), void>::type
		give(DataStore::Handle<cid, L, HA, T> &hndl, std::unique_ptr<T> obj) 
			{hndl.hi() = std::move(obj);}
		
		template<Level L, typename T, HandleAccess HA>
		typename std::enable_if<canWrite(HA), void>::type
		give(DataStore::Handle<cid, L, HA, T> &hndl, T* obj) 
			{hndl.hi() = std::unique_ptr<T>(obj);}
		
		template<Level L, typename T>
		std::unique_ptr<T> take(DataStore::Handle<cid, L,HandleAccess::all,T>& hndl)
			{ return hndl.hi();}
		
		//commutative operations
		
			
		template<Level L, typename T, HandleAccess HA>
		typename std::enable_if<canWrite(HA), void>::type
		incr_op(DataStore::Handle<cid, L, HA, T> &h) 
			{
				auto f = [&](){(*(h.hi().stored_obj))++;};
				pending_updates.push_back(f);
				f();				
			}
		
		template<Level L, typename T, HandleAccess HA>
		typename std::enable_if<canWrite(HA), void>::type
		incr(DataStore::Handle<cid, L, HA, T> &h) 
			{h.hi().stored_obj->incr();}
		
		template<Level L, typename T, HandleAccess HA, typename... A>
		typename std::enable_if<canRead(HA), void>::type
		add(DataStore::Handle<cid, L, HA, T> &h, A... args) 
			{h.hi().stored_obj->add(args...);}
		
		//transactions interface
		
		template < typename R, typename... Args>
		auto ro_transaction(R &f, Args... args) {
			static_assert(all_handles<Args...>::value, "Passed non-DataStore::Handles as arguments to function!");
			static_assert(!exists_write_handle<Args...>::value, "Passed write-enabled handles as argument to ro function!");
			static_assert(is_stateless<R, Client&, Args...>::value,
				      "You passed me a non-stateless function, or screwed up your arguments! \n Expected: R f(DataStore&, DataStore::Handles....)");
			static_assert(all_handles_read<Args...>::value, "Error: passed non-readable handle into ro_transaction");
			return f(*this, args...);
		}
		
		
		template < typename R, typename... Args>
		auto wo_transaction(R &f, Args... args) {
			static_assert(all_handles<Args...>::value, "Passed non-DataStore::Handles as arguments to function!");
			static_assert(!exists_read_handle<Args...>::value, "Passed read-enabled handles as argument to wo function!");
			static_assert(is_stateless<R, Client&, Args...>::value,
				      "You passed me a non-stateless function, or screwed up your arguments! \n Expected: R f(DataStore&, DataStore::Handles....)");
			static_assert(all_handles_write<Args...>::value, "Error: passed non-writeable handle into wo_transaction");
			return f(*this, args...);
		}
		
		template < typename R, typename... Args>
		auto rw_transaction(R &f, Args... args) {
			static_assert(all_handles<Args...>::value, "Passed non-DataStore::Handles as arguments to function!");
			static_assert(is_stateless<R, Client&, Args...>::value,
				      "You passed me a non-stateless function, or screwed up your arguments! \n Expected: R f(DataStore&, DataStore::Handles....)");
			static_assert(all_handles_write<Args...>::value, "Error: passed non-writeable handle into rw_transaction");
			static_assert(all_handles_read<Args...>::value, "Error: passed non-readable handle into rw_transaction");
			return f(*this, args...);
		}

		typedef void (*copy_hndls_f) (DataStore& from, DataStore &to);
		
		void waitForSync(){
			static const copy_hndls_f copy_hndls = [](DataStore& from, DataStore &to){
				for (auto& ptr_p : to.hndls) {
					auto &ptr = ptr_p.second;
					auto &m_ptr = from.hndls[ptr->id];
					if (m_ptr->rid == ptr->rid) {
						ptr.operator=(m_ptr->clone(to));
					}
				}
			};
			copy_hndls(master,local);
			for (auto &update : pending_updates) update();
			copy_hndls(local,master);
			pending_updates.clear();
		}

		template<Client_Id>
		friend class Client;


	};
	
	

};
