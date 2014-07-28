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
	enum class Level { causal, strong};
}

namespace tester{	
	template<backend::Level L, typename R, typename IR>
	class Fuzz;
}

namespace backend{

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

	static constexpr bool operator <= (HandleAccess h1, HandleAccess h2){
		return ((int)h1) <= ((int) h2);
	}

	static constexpr bool operator >= (HandleAccess h1, HandleAccess h2){
		return ((int)h1) >= ((int) h2);
	}


	class DataStore {

	public:

		//So that you can store lists of these things, if you want.
		class GenericHandle;

		template<typename T>
		class TypedHandle; //extends GenericHandle
		


		template<Level L, HandleAccess HA, typename T>
		class Handle; //extends TypedHandle<T>

//hiding implemntation details here.  
#include "Backend-impl.h"

		//create/delete object slots

		template<Level L, typename T>
		Handle<L,HandleAccess::all, T> 
		newHandle(std::unique_ptr<T> r)
			{return newhandle_internal<L,HandleAccess::all>
					(std::move(r));}

		template<Level L, typename T>
		Handle<L,HandleAccess::all, T> newHandle(T r)
			{return newhandle_internal<L,HandleAccess::all>
					(std::unique_ptr<T>(new T(r)));}

		template<Level L, typename T>
		Handle<L,HandleAccess::all, T> newHandle(T* r = nullptr)
			{return newhandle_internal<L,HandleAccess::all>
					(std::unique_ptr<T>(r));}

		template<Level L,typename T>
		std::unique_ptr<T> del(Handle<L,HandleAccess::all,T>& hndl)
			{return del_internal<L>(hndl);}

		template<Level L, typename T>
		auto ro_hndl(Handle<L,HandleAccess::all,T> &old){
			return Handle<L,HandleAccess::read,T>(old.hi());
		}

		template<Level L, typename T>
		auto wo_hndl(Handle<L,HandleAccess::all,T> &old){
			return Handle<L,HandleAccess::write,T>(old.hi());
		}

		static constexpr HandleAccess newConsistency_lvl(Level Lold){
			return (Lold == Level::strong ? HandleAccess::read : HandleAccess::write);
		}

		template<Level Lnew, Level Lold, typename T>
		auto newConsistency
		(Handle<Lold,HandleAccess::all,T> &old) {
		return Handle<Lnew,newConsistency_lvl(Lold), T> (old.hi());
		}


		//KVstore-style interface

		template<Level L, typename T, HandleAccess HA>
		typename std::enable_if<canRead(HA), T&>::type
		get(Handle<L, HA, T> &hndl)
			{return hndl.hi();}

		template<Level L, typename T, HandleAccess HA>
		typename std::enable_if<canWrite(HA), void>::type
		give(Handle<L, HA, T> &hndl, std::unique_ptr<T> obj) 
			{hndl.hi() = std::move(obj);}

		template<Level L, typename T, HandleAccess HA>
		typename std::enable_if<canWrite(HA), void>::type
		give(Handle<L, HA, T> &hndl, T* obj) 
			{hndl.hi() = std::unique_ptr<T>(obj);}

		template<Level L, typename T>
		std::unique_ptr<T> take(Handle<L,HandleAccess::all,T>& hndl)
			{ return hndl.hi();}

		//commutative operations


		template<Level L, typename T, HandleAccess HA>
		typename std::enable_if<canWrite(HA), void>::type
		incr_op(Handle<L, HA, T> &h) 
			{(*(h.hi().stored_obj))++;}

		template<Level L, typename T, HandleAccess HA>
		typename std::enable_if<canWrite(HA), void>::type
		incr(Handle<L, HA, T> &h) 
			{h.hi().stored_obj->incr();}

		template<Level L, typename T, HandleAccess HA, typename... A>
		typename std::enable_if<canRead(HA), void>::type
		add(Handle<L, HA, T> &h, A... args) 
			{h.hi().stored_obj->add(args...);}

		//transactions interface

		template < typename R, typename... Args>
		auto ro_transaction(R &f, Args... args) {
			static_assert(all_handles<Args...>::value, "Passed non-Handles as arguments to function!");
			static_assert(!exists_write_handle<Args...>::value, "Passed write-enabled handles as argument to ro function!");
			static_assert(is_stateless<R, DataStore&, Args...>::value,
				      "You passed me a non-stateless function, or screwed up your arguments! \n Expected: R f(DataStore&, Handles....)");
			static_assert(all_handles_read<Args...>::value, "Error: passed non-readable handle into ro_transaction");
			return f(*this, args...);
		}


		template < typename R, typename... Args>
		auto wo_transaction(R &f, Args... args) {
			static_assert(all_handles<Args...>::value, "Passed non-Handles as arguments to function!");
			static_assert(!exists_read_handle<Args...>::value, "Passed read-enabled handles as argument to wo function!");
			static_assert(is_stateless<R, DataStore&, Args...>::value,
				      "You passed me a non-stateless function, or screwed up your arguments! \n Expected: R f(DataStore&, Handles....)");
			static_assert(all_handles_write<Args...>::value, "Error: passed non-writeable handle into wo_transaction");
			return f(*this, args...);
		}

		template < typename R, typename... Args>
		auto rw_transaction(R &f, Args... args) {
			static_assert(all_handles<Args...>::value, "Passed non-Handles as arguments to function!");
			static_assert(is_stateless<R, DataStore&, Args...>::value,
				      "You passed me a non-stateless function, or screwed up your arguments! \n Expected: R f(DataStore&, Handles....)");
			static_assert(all_handles_write<Args...>::value, "Error: passed non-writeable handle into rw_transaction");
			static_assert(all_handles_read<Args...>::value, "Error: passed non-readable handle into rw_transaction");
			return f(*this, args...);
		}

		//constructors and destructor

		DataStore () {}
		DataStore (const DataStore &) = delete;
		virtual ~DataStore() {
			this-> destructing = true;
		}
		
		friend class HandlePrime;
		
		template<Level L, typename R, typename IR>
		friend class tester::Fuzz;
	};


};
