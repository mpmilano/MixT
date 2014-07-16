#pragma once
#include <list>
#include <memory>
#include <vector>
#include <queue>
#include <cassert>
#include <iostream>

namespace backend {
	enum class Level { causal, strong, fastest};

	template <Level L>
	class DataStore {

	public:
		template<typename T>
		class Handle;

//hiding implemntation details here.  
#include "Backend-impl.h"

		//create/delete object slots

		template<typename T>
		Handle<T>& newHandle(std::unique_ptr<T> r){return newhandle_internal(std::move(r));}

		template<typename T>
		Handle<T>& newHandle(T r){return newhandle_internal(std::unique_ptr<T>(new T(r)));}

		template<typename T>
		Handle<T>& newHandle(T* r = nullptr){return newhandle_internal(std::unique_ptr<T>(r));}

		template<typename T>
		std::unique_ptr<T> del(Handle<T>& hndl) {return del_internal(hndl);}

		//KVstore-style interface

		template<typename T>
		T& get(Handle<T> &hndl) {return hndl;}

		template<typename T>
		void give(Handle<T> &hndl, std::unique_ptr<T> obj) {hndl = std::move(obj);}

		template<typename T>
		void give(Handle<T> &hndl, T* obj) {hndl = std::unique_ptr<T>(obj);}
		
		template<typename T>
		std::unique_ptr<T> take(Handle<T>& hndl){ return hndl;}

		//commutative operations

		template<typename T>
		void incr_op(Handle<T> &h) {h.stored_obj->operator++();}

		template<typename T>
		void incr(Handle<T> &h) {h.stored_obj->incr();}

		template<typename T, typename... A>
		void add(Handle<T> &h, A... args) {h.stored_obj->add(args...);}

		template<typename T, typename F, typename... A>
		void add_f(Handle<T> &h, F addfun, A... args) {F(*(h.stored_obj), args...);}

		//constructors and destructor

		DataStore () {}
		DataStore (const DataStore<L> &) = delete;
		virtual ~DataStore() {
			this-> destructing = true;
		}
		
		friend class HandlePrime;
	};


};
