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
	enum class Level { causal, strong, fastest};
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
	
	template<typename... Rest>
	static constexpr bool canRead(HandleAccess ha, Rest... r){
		return ha == HandleAccess::read ? canRead(r...)
			: (ha == HandleAccess::all ? 
			   canRead (r... ) : false);
	}

	static constexpr bool canRead(HandleAccess ha){
		return ha == HandleAccess::read ? true
			: (ha == HandleAccess::all ? 
			   true : false);
	}

	class DataStore {

	private: 
		Level fastest_lvl = Level::fastest;

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


		template<typename... Args>
		struct all_handles : std::conditional<
			any <is_not_handle, pack<Args...> >::value,
			std::integral_constant<bool,false>,
			std::integral_constant<bool,true>>::type {};

		template < typename R, typename... Args>
		auto ro_transaction(R &f, Args... args) {
			static_assert(all_handles<Args...>::value, "Passed non-Handles as arguments to function!");
			static_assert(is_stateless<R, DataStore&, Args...>::value,
				      "You passed me a non-stateless function! \n Expected: R f(DataStore&, Handles....)");
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
