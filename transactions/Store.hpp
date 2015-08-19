#pragma once
#include "utils.hpp"

//TODO: define this better and move it.
struct Store : std::map<int,std::unique_ptr<void*> >{
	//you are *not* responsible for deleting this
	Store * const  prev_scope = nullptr;
	bool contains(int i) const{
		return this->find(i) != this->end();
	}

	typedef void** stored;

	Store(){}

	Store(Store  * const prev):prev_scope(prev){}

	Store(const Store&) = delete;

	template<typename T>
	auto insert(int i, const T &item) {
		return (*this)[i].reset((stored)heap_copy(item));
	}

	template<typename T>
	auto emplace(int i){
		return (*this)[i].reset((void**)new T());
	}

	template<typename T>
	T& get(int i){
		if (!contains(i) && prev_scope)
			return prev_scope->get<T>(i);
		T* ret = (T*) (this->at(i).get());
		assert(ret);
		return *ret;
	}

	template<typename T>
	const T& get(int i) const{
		if (!contains(i) && prev_scope)
			return prev_scope->get<T>(i);
		T* ret = (T*) (this->at(i).get());
		assert(ret);
		return *ret;
	}

};

Store& mke_store(){
	static Store s;
	return s;
}
