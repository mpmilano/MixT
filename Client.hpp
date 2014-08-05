#pragma once
#include "Backend.hpp"
#include <execinfo.h>

namespace backend {
	
	template<Client_Id cid>
	class Client {

		//BEGIN INTERNALS
	private:

		DataStore& master;
		DataStore local;
		
		bool sync_enabled = true;

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
				pending_lock(pending& cl):cl(cl){
					cl.locked = true;
				}
			public:
				~pending_lock(){
					cl.locked = false;
				}
				friend class pending;
			};
			friend class pending_lock;
			pending_lock lock(){return pending_lock(*this);}
		};

		pending pending_updates;

		template<Level L, typename T, Level _L, HandleAccess _ha, Client_Id _cid>
		DataStore::Handle<cid,L,HandleAccess::all, T>
		gethandle_internal(const DataStore::Handle<_cid,_L,_ha,T> &underlying){
			return local.get_handle<cid,L>(underlying.hi());
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

		//END INTERNALS
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

		
		//KVstore-style interface

		template<Level L, typename T, HandleAccess HA>
		typename std::enable_if<canRead(HA), T&>::type
		get(DataStore::Handle<cid,L, HA, T> &hndl) {
			waitForSync<L>(); return hndl.hi();
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
			waitForSync<L>();
		}
		
		template<Level L, typename T, HandleAccess HA>
		typename std::enable_if<canWrite(HA), void>::type
		give(DataStore::Handle<cid, L, HA, T> &hndl, T* obj) {
			pending_updates.run([&] (typename pending::push_f &push ){
					std::shared_ptr<T> cpy(new T(*obj),release_deleter<T>());
					upfun &&tmp = [hndl,cpy](){
						std::get_deleter<release_deleter<T> >(cpy)->release();
						hndl.hi() = std::unique_ptr<T>(cpy.get());};
					push(tmp);
				});
			hndl.hi() = std::unique_ptr<T>(obj);
			waitForSync<L>();
		}
		
		template<Level L, typename T>
		std::unique_ptr<T> take(DataStore::Handle<cid, L,HandleAccess::all,T>& hndl) {
			waitForSync<L>();
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
				waitForSync<L>();		
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
		
		template<Level L, typename T, HandleAccess HA, typename F, typename... A>
		typename std::enable_if<canRead(HA), void>::type
		add(DataStore::Handle<cid, L, HA, T> &h, F f2, A... args) {
			//todo - lifetime of args?
			upfun f = [h,f2,args...](){ f2(h.hi().stored_obj.get(),args...);};
			pending_updates.run([&f](typename pending::push_f &push){
					push(f);
				});
			f();
			waitForSync<L>();
		}
		
		friend class transaction_cls;
		//transactions interface
		class transaction_cls {
		private:
			bool sync = false;
			Client &c;
			transaction_cls(Client& c):c(c){
				c.sync_enabled = false;
			}

			template<Level l>
			void waitForSync(){
				c.sync_enabled = true;
				c.waitForSync<l>();
				c.sync_enabled = false;
			}
		public:
			
			template < typename R, typename... Args>
			auto ro(R &f, Args... args) {
				static_assert(all_handles<Args...>::value, "Passed non-DataStore::Handles as arguments to function!");
				static_assert(!exists_write_handle<Args...>::value, "Passed write-enabled handles as argument to ro function!");
				static_assert(is_stateless<R, Client&, Args...>::value,
					      "You passed me a non-stateless function, or screwed up your arguments! \n Expected: R f(DataStore&, DataStore::Handles....)");
				static_assert(all_handles_read<Args...>::value, "Error: passed non-readable handle into ro_transaction");
				waitForSync<any_required_sync<Args...>::value ? Level::strong : Level::causal>();
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
				if (any_required_sync<Args...>::value) sync = true;
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
				if (any_required_sync<Args...>::value) {
					sync = true;
					waitForSync<Level::strong>();
				} else waitForSync<Level::causal>();
				c.pending_updates.run([&](typename pending::push_f &push){push(f3);});
				auto l = c.pending_updates.lock();
				return f2(c, args...);
			}
			~transaction_cls(){
				c.sync_enabled = true;
				if (sync) c.waitForSync<Level::strong>();
				else c.waitForSync<Level::causal>();
			}
			friend class Client;
		};
		
		transaction_cls transaction(){ return transaction_cls(*this); }
		
		template<Level l>
		void waitForSync(){
			if (!sync_enabled) return;
			if (l != Level::strong) return;
			sync_enabled = false;
			master.syncClient(local);
			pending_updates.runAndClear();
			local.syncClient(master);
			assert(pending_updates.isClear());
			sync_enabled = true;
		}

		template<Client_Id>
		friend class Client;
	};
}
