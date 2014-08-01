#pragma once
#include "Backend.hpp"
#include <execinfo.h>

namespace backend {
	
	template<Client_Id cid>
	class Client {
	private:

		DataStore& master;
		DataStore local;
		
		typedef std::function<void ()> upfun;
		class pending{
		private:
			bool locked = false;
			std::list<upfun> pending_updates;
			void push(upfun up){
				if (!locked) pending_updates.push_back(std::move(up));
			}
			std::function<void (upfun&) > push_ = [&](upfun &f){ push(f);};

		public:
			pending(const pending&) = delete;
			pending(pending &&p):locked(p.locked),
					     pending_updates(std::move(p.pending_updates)),
					     push_(std::move(p.push_)){}
			pending(){}
			void runAndClear(){
				if (!locked){
					auto l = lock();
					for (auto &up : pending_updates) up();
					pending_updates.clear();
				}
			}

			bool isClear(){
				return pending_updates.size() == 0;
			}

			typedef std::function<void (upfun&)> push_f;
			
			void run(std::function<void (push_f&) > &&f){
				if (!locked) f(push_);
			}

			class pending_lock{
				
			private:
				pending& cl;
				pending_lock(pending& cl):cl(cl){cl.locked = true;}
			public:
				~pending_lock(){cl.locked = false;}
				friend class pending;
			};
			friend class pending_lock;
			pending_lock lock(){return pending_lock(*this);}
		};

		pending pending_updates;

		template<Level L, typename T>
		DataStore::Handle<cid,L,HandleAccess::all, T>
		gethandle_internal(const DataStore::HandleImpl<T> &underlying){
			assert(local.hndls[underlying.id].get() == nullptr );
			assert(master.hndls[underlying.id].get() != nullptr );
			assert(master.hndls[underlying.id]->rid == underlying.rid);
			return DataStore::Handle<cid,L,HandleAccess::all,T>(underlying.clone(local));
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
		std::unique_ptr<T> del(DataStore::Handle<cid, L,HandleAccess::all,T>& hndl){
			if (L == Level::strong) waitForSync();
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
		DataStore::Handle<cid,l,ha,T> get_access(DataStore::Handle<cid_old,l,ha,T> &hndl, Client<cid_old> &o) {
			static_assert(cid != cid_old, "You already have access to this handle.");
			assert (&master == &o.master);
			return gethandle_internal<l>(hndl.hi());
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
		give(DataStore::Handle<cid, L, HA, T> &hndl, std::unique_ptr<T> obj) {
			pending_updates.run([&] (typename pending::push_f &push) {
					std::shared_ptr<T> cpy(new T(*obj),release_deleter<T>());
					push([hndl,cpy](){
							std::get_deleter<release_deleter<T> >(cpy)->release();
							hndl.hi() = std::unique_ptr<T>(cpy.get());});	
				});
			hndl.hi() = std::move(obj);
			if (L == Level::strong) waitForSync();
		}
		
		template<Level L, typename T, HandleAccess HA>
		typename std::enable_if<canWrite(HA), void>::type
		give(DataStore::Handle<cid, L, HA, T> &hndl, T* obj) {
			pending_updates.run([&] (typename pending::push_f &push ){
					std::shared_ptr<T> cpy(new T(*obj),release_deleter<T>());
					push([hndl,cpy](){
							std::get_deleter<release_deleter<T> >(cpy)->release();
							hndl.hi() = std::unique_ptr<T>(cpy.get());});
				});
			hndl.hi() = std::unique_ptr<T>(obj);
			if (L == Level::strong) waitForSync();
		}
		
		template<Level L, typename T>
		std::unique_ptr<T> take(DataStore::Handle<cid, L,HandleAccess::all,T>& hndl) {
			if (L == Level::strong) waitForSync();
			pending_updates.run([&hndl](typename pending::push_f &push) {
					push([hndl](){hndl.hi().reset();});});
			return hndl.hi();
		}
		
		//commutative operations
		
			
		template<Level L, typename T, HandleAccess HA>
		typename std::enable_if<canWrite(HA), void>::type
		incr_op(DataStore::Handle<cid, L, HA, T> &h) 
			{
				upfun f = [h](){(*(h.hi().stored_obj))++;};
				std::function<void (typename pending::push_f&)> pf = 
					[&f](typename pending::push_f &push){push(f);};
				pending_updates.run(std::move(pf));
				f();
				if (L == Level::strong) waitForSync();		
			}
		
		template<Level L, typename T, HandleAccess HA>
		typename std::enable_if<canWrite(HA), void>::type
		incr(DataStore::Handle<cid, L, HA, T> &h) {
			upfun f = [h](){ h.hi().stored_obj->incr(); };
			pending_updates.run([&f](typename pending::push_f &push){
					push(f);
				});
			f();
		}
		
		template<Level L, typename T, HandleAccess HA, typename... A>
		typename std::enable_if<canRead(HA), void>::type
		add(DataStore::Handle<cid, L, HA, T> &h, A... args) {
			//todo - lifetime of args?
			upfun f = [h,args...](){ h.hi().stored_obj->add(args...);};
			pending_updates.run([&f](typename pending::push_f &push){
					push(f);
				});
			f();
			if (L == Level::strong) waitForSync();
		}
		
		//transactions interface
		class transaction_cls {
		private:
			bool sync = false;
			Client &c;
			transaction_cls(Client& c):c(c){}
		public:
			
			template < typename R, typename... Args>
			auto ro(R &f, Args... args) {
				static_assert(all_handles<Args...>::value, "Passed non-DataStore::Handles as arguments to function!");
				static_assert(!exists_write_handle<Args...>::value, "Passed write-enabled handles as argument to ro function!");
				static_assert(is_stateless<R, Client&, Args...>::value,
					      "You passed me a non-stateless function, or screwed up your arguments! \n Expected: R f(DataStore&, DataStore::Handles....)");
				static_assert(all_handles_read<Args...>::value, "Error: passed non-readable handle into ro_transaction");
				return f(c, args...);
			}
			
			template < typename R, typename... Args>
			void wo(R &f, Args... args) {
				static_assert(all_handles<Args...>::value, "Passed non-DataStore::Handles as arguments to function!");
				static_assert(!exists_read_handle<Args...>::value, "Passed read-enabled handles as argument to wo function!");
				static_assert(is_stateless<R, Client&, Args...>::value,
					      "You passed me a non-stateless function, or screwed up your arguments! \n Expected: R f(DataStore&, DataStore::Handles....)");
				static_assert(all_handles_write<Args...>::value, "Error: passed non-writeable handle into wo_transaction");
				typename funcptr<R, Client&, Args...>::type f2 = f;
				static upfun f3 = [this,f2,args...](){
					f2(c,args...);
				};
				c.pending_updates.run([&](typename pending::push_f &push){push(f3);});
				auto l = c.pending_updates.lock();
				f2(c, args...);
			}
			
			template < typename R, typename... Args>
			auto rw(R &f, Args... args) {
				static_assert(all_handles<Args...>::value, "Passed non-DataStore::Handles as arguments to function!");
				static_assert(is_stateless<R, Client&, Args...>::value,
					      "You passed me a non-stateless function, or screwed up your arguments! \n Expected: R f(DataStore&, DataStore::Handles....)");
				static_assert(all_handles_write<Args...>::value, "Error: passed non-writeable handle into rw_transaction");
				static_assert(all_handles_read<Args...>::value, "Error: passed non-readable handle into rw_transaction");
				typename funcptr<R, Client&, Args...>::type f2 = f;
				static upfun f3 = [this,f2,args...](){
					f2(c,args...);
				};
				c.pending_updates.run([&](typename pending::push_f &push){push(f3);});
				auto l = c.pending_updates.lock();
				return f2(c, args...);
			}
			~transaction_cls(){ if (sync) c.waitForSync();}
			friend class Client;
		};
		
		transaction_cls transaction(){ return transaction_cls(*this); }
		
		void waitForSync(){
			//std::cout << "sync requested!" << std::endl;
			typedef void (*copy_hndls_f) (DataStore& from, DataStore &to);
			static const copy_hndls_f copy_hndls = [](DataStore& from, DataStore &to){
				for (auto& ptr_p : to.hndls) {
					auto &ptr = ptr_p.second;
					auto &m_ptr = from.hndls[ptr->id];
					if (m_ptr->rid == ptr->rid) {
						ptr->grab_obj(*m_ptr);
					}
				}
			};
			copy_hndls(master,local);
			pending_updates.runAndClear();
			copy_hndls(local,master);
			assert(pending_updates.isClear());
		}

		template<Client_Id>
		friend class Client;


	};
	
	

};
