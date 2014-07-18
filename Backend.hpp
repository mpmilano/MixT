#pragma once
#include <list>
#include <memory>
#include <vector>
#include <queue>
#include <cassert>
#include <iostream>

namespace backend {
	enum class Level { causal, strong, fastest};

	class DataStore {

	public:

		//So that you can store lists of these things, if you want.
		class GenericHandle;

		template<typename T>
		class TypedHandle; //extends GenericHandle
		
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

		template<Level Lnew, typename T>
		Handle<Lnew, HandleAccess::read, T> newConsistency
		(Handle<Level::strong,HandleAccess::all,T> &old)
			{return Handle<Lnew,HandleAccess::read,T>(old.hi());}

		template<Level Lnew, typename T>
		Handle<Lnew, HandleAccess::write, T> newConsistency
		(Handle<Level::causal,HandleAccess::all,T> &old)
			{ return Handle<Lnew,HandleAccess::write, T>
					(old.hi());}


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

		//constructors and destructor

		DataStore () {}
		DataStore (const DataStore &) = delete;
		virtual ~DataStore() {
			this-> destructing = true;
		}
		
		friend class HandlePrime;
	};


};
