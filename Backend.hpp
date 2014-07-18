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

		template<Level L, typename T>
		T& get(Handle<L, HandleAccess::read, T> &hndl)
			{return hndl.hi();}

		template<Level L, typename T>
		T& get(Handle<L, HandleAccess::all, T> &hndl) 
			{return hndl.hi();}

		template<Level L, typename T>
		void give(Handle<L, HandleAccess::write, T> &hndl, 
			  std::unique_ptr<T> obj) 
			{hndl.hi() = std::move(obj);}

		template<Level L, typename T>
		void give(Handle<L, HandleAccess::all, T> &hndl, 
			  std::unique_ptr<T> obj) 
			{hndl.hi() = std::move(obj);}

		template<Level L, typename T>
		void give(Handle<L, HandleAccess::write, T> &hndl, T* obj) 
			{hndl.hi() = std::unique_ptr<T>(obj);}

		template<Level L, typename T>
		void give(Handle<L, HandleAccess::all, T> &hndl, T* obj) 
			{hndl.hi() = std::unique_ptr<T>(obj);}

		template<Level L, typename T>
		std::unique_ptr<T> take(Handle<L,HandleAccess::all,T>& hndl)
			{ return hndl.hi();}

		//commutative operations

		template<Level L, typename T>
		void incr_op(Handle<L, HandleAccess::all, T> &h) 
			{(*(h.hi().stored_obj))++;}
		template<Level L, typename T>
		void incr_op(Handle<L, HandleAccess::write, T> &h) 
			{(*(h.hi().stored_obj))++;}

		template<Level L, typename T>
		void incr(Handle<L, HandleAccess::write, T> &h) 
			{h.hi().stored_obj->incr();}
		template<Level L, typename T>
		void incr(Handle<L, HandleAccess::all, T> &h) 
			{h.hi().stored_obj->incr();}

		template<Level L, typename T, typename... A>
		void add(Handle<L, HandleAccess::write, T> &h, A... args) 
			{h.hi().stored_obj->add(args...);}
		template<Level L, typename T, typename... A>
		void add(Handle<L, HandleAccess::all, T> &h, A... args) 
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
