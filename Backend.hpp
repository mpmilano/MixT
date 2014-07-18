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
		template<Level L, typename T>
		class Handle;

//hiding implemntation details here.  
#include "Backend-impl.h"

		//create/delete object slots

		template<Level L, typename T>
		Handle<L, T> newHandle(std::unique_ptr<T> r){return newhandle_internal<L>(std::move(r));}

		template<Level L, typename T>
		Handle<L, T> newHandle(T r){return newhandle_internal<L>(std::unique_ptr<T>(new T(r)));}

		template<Level L, typename T>
		Handle<L, T> newHandle(T* r = nullptr){return newhandle_internal<L>(std::unique_ptr<T>(r));}

		template<Level L, typename T>
		std::unique_ptr<T> del(Handle<L, T>& hndl) {return del_internal<L>(hndl);}

		template<Level Lnew, Level Lold, typename T>
		Handle<Lnew, T> newConsistency(Handle<Lold, T> &old){ return Handle<Lnew, T>(old.hi);}

		//KVstore-style interface

		template<Level L, Level L_effective = L, typename T>
		T& get(Handle<L, T> &hndl) {return hndl.hi;}

		template<Level L, Level L_effective = L, typename T>
		void give(Handle<L, T> &hndl, std::unique_ptr<T> obj) {hndl.hi = std::move(obj);}

		template<Level L, Level L_effective = L, typename T>
		void give(Handle<L, T> &hndl, T* obj) {hndl.hi = std::unique_ptr<T>(obj);}
		
		template<Level L, Level L_effective = L, typename T>
		std::unique_ptr<T> take(Handle<L, T>& hndl){ return hndl.hi;}

		//commutative operations

		template<Level L, Level L_effective = L, typename T>
		void incr_op(Handle<L, T> &h) {h.hi.stored_obj->operator++();}

		template<Level L, Level L_effective = L, typename T>
		void incr(Handle<L, T> &h) {h.hi.stored_obj->incr();}

		template<Level L, Level L_effective = L, typename T, typename... A>
		void add(Handle<L, T> &h, A... args) {h.hi.stored_obj->add(args...);}

		template<Level L, Level L_effective = L, typename T, typename F, typename... A>
		void add_f(Handle<L, T> &h, F addfun, A... args) {F(*(h.hi.stored_obj), args...);}

		//constructors and destructor

		DataStore () {}
		DataStore (const DataStore &) = delete;
		virtual ~DataStore() {
			this-> destructing = true;
		}
		
		friend class HandlePrime;
	};


};
