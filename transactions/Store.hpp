#pragma once
#include "utils.hpp"

//TODO: define this better and move it.
struct Store : std::map<int,std::unique_ptr<void*> >{
	//you are *not* responsible for deleting this
	Store * const  prev_scope = nullptr;
	bool valid_store = true;
	bool contains(int i) const{
		return this->find(i) != this->end();
	}

	typedef void** stored;

	Store(){}

	Store(Store  * const prev):prev_scope(prev){}

	Store(const Store&) = delete;

	template<typename T>
	auto insert(int i, const T &item) {
		assert(valid_store);
		return (*this)[i].reset((stored)heap_copy(item));
	}

	template<typename T>
	auto emplace(int i){
		assert(valid_store);
		return (*this)[i].reset((void**)new T());
	}

	template<typename T>
	T& get(int i){
		if (!contains(i) && prev_scope)
			return prev_scope->get<T>(i);
		else if (!contains(i)) assert(false && "Error: we don't have that here");
		T* ret = (T*) (this->at(i).get());
		assert(ret);
		return *ret;
	}

	template<typename T>
	const T& get(int i) const{
		if (!contains(i) && prev_scope)
			return prev_scope->get<T>(i);
		else if (!contains(i)) assert(false && "Error: we don't have that here");
		T* ret = (T*) (this->at(i).get());
		assert(ret);
		return *ret;
	}

	virtual ~Store() {
		valid_store = false;
		std::cout << "destroying store" << std::endl;
	}

};

