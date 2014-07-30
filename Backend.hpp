#pragma once
#include <list>
#include <memory>
#include <vector>
#include <queue>
#include <cassert>
#include <iostream>
#include "extras"

#define LVALUE(x) typename add_lvalue_reference<x>::type

namespace backend {
	//"strong" is Top here.  Linearizable, + start-time ordered
	//"causal" is GLUT.
	enum class Level { causal, strong};
}

namespace tester{	
	template<backend::Level L, typename R, typename IR>
	class Fuzz;
}

namespace backend{

	class Client;

	enum class HandleAccess {read, write, all};
	
	static constexpr bool canWrite(HandleAccess ha){
		return ha == HandleAccess::write ? true 
			: (ha == HandleAccess::all ? 
			   true : false);
	}

	static constexpr bool canRead(HandleAccess ha){
		return ha == HandleAccess::read ? true
			: (ha == HandleAccess::all ? 
			   true : false);
	}

	class DataStore {

	public:

		//So that you can store lists of these things, if you want.
		class GenericHandle;

		template<typename T>
		class TypedHandle; //extends GenericHandle
		


		template<Level L, HandleAccess HA, typename T>
		class Handle; //extends TypedHandle<T>

	private:

		void syncClient(DataStore&){
			
		}

//hiding implemntation details here.  
#include "Backend-impl.h"

		//constructors and destructor
		DataStore () {}

		DataStore(DataStore&& ds):
			hndls(std::move(ds.hndls)),
			next_ids(std::move(ds.next_ids)),
			destructing(std::move(ds.destructing)){}

		DataStore (const DataStore &) = delete;
		virtual ~DataStore() {
			this-> destructing = true;
		}
		
		friend class HandlePrime;
		friend class Client;
		
		template<Level L, typename R, typename IR>
		friend class tester::Fuzz;
	};

}

//template magic in here.
#include "handle_utils"

namespace backend {
	
	class Client {
	private:
		DataStore& master;
		DataStore local;
	public:
		
		Client(DataStore& master):master(master){
			master.syncClient(local);
		}

		Client(Client&& old):master(old.master),local(std::move(old.local)){}
		
		//create/delete object slots
		
		template<Level L, typename T>
		DataStore::Handle<L,HandleAccess::all, T>
		newHandle(std::unique_ptr<T> r)
			{return local.newhandle_internal<L,HandleAccess::all>
					(std::move(r));}
		
		template<Level L, typename T>
		DataStore::Handle<L,HandleAccess::all, T> newHandle(T r)
			{return local.newhandle_internal<L,HandleAccess::all>
					(std::unique_ptr<T>(new T(r)));}
		
		template<Level L, typename T>
		DataStore::Handle<L,HandleAccess::all, T> newHandle(T* r = nullptr)
			{return local.newhandle_internal<L,HandleAccess::all>
					(std::unique_ptr<T>(r));}
		
		template<Level L,typename T>
		std::unique_ptr<T> del(DataStore::Handle<L,HandleAccess::all,T>& hndl)
			{return local.del_internal<L>(hndl);}
		
		template<Level L, typename T>
		auto ro_hndl(DataStore::Handle<L,HandleAccess::all,T> &old){
			return DataStore::Handle<L,HandleAccess::read,T>(old.hi());
		}
		
		template<Level L, typename T>
		auto wo_hndl(DataStore::Handle<L,HandleAccess::all,T> &old){
			return DataStore::Handle<L,HandleAccess::write,T>(old.hi());
		}
		
		static constexpr HandleAccess newConsistency_lvl(Level Lold){
			auto ret =(Lold == Level::strong ? HandleAccess::read : HandleAccess::write);
			return ret;
		}
		
		template<Level Lnew, Level Lold, typename T>
		auto newConsistency (DataStore::Handle<Lold,HandleAccess::all,T> &old) {
			return DataStore::Handle<Lnew,newConsistency_lvl(Lold), T> (old.hi());
		}
			
		
		template<Level L, typename T, HandleAccess HA>
		void waitForSync(DataStore::Handle<L, HA, T> &){}
		
		//KVstore-style interface
		
		template<Level L, typename T, HandleAccess HA>
		typename std::enable_if<canRead(HA), T&>::type
		get(DataStore::Handle<L, HA, T> &hndl)
			{return hndl.hi();}
		
		template<Level L, typename T, HandleAccess HA>
		typename std::enable_if<canWrite(HA), void>::type
		give(DataStore::Handle<L, HA, T> &hndl, std::unique_ptr<T> obj) 
			{hndl.hi() = std::move(obj);}
		
		template<Level L, typename T, HandleAccess HA>
		typename std::enable_if<canWrite(HA), void>::type
		give(DataStore::Handle<L, HA, T> &hndl, T* obj) 
			{hndl.hi() = std::unique_ptr<T>(obj);}
		
		template<Level L, typename T>
		std::unique_ptr<T> take(DataStore::Handle<L,HandleAccess::all,T>& hndl)
			{ return hndl.hi();}
		
		//commutative operations
		
			
		template<Level L, typename T, HandleAccess HA>
		typename std::enable_if<canWrite(HA), void>::type
		incr_op(DataStore::Handle<L, HA, T> &h) 
			{(*(h.hi().stored_obj))++;}
		
		template<Level L, typename T, HandleAccess HA>
		typename std::enable_if<canWrite(HA), void>::type
		incr(DataStore::Handle<L, HA, T> &h) 
			{h.hi().stored_obj->incr();}
		
		template<Level L, typename T, HandleAccess HA, typename... A>
		typename std::enable_if<canRead(HA), void>::type
		add(DataStore::Handle<L, HA, T> &h, A... args) 
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
	};
	
	

};
