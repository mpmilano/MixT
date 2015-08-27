#pragma once
#include "utils.hpp"

//TODO: define this better and move it.
struct Store {

	std::map<int,std::unique_ptr<void*> > store_impl;
	
	//you are *not* responsible for deleting this
	Store * const  prev_scope = nullptr;
	bool valid_store = true;
	bool contains(int i) const{
		return (store_impl.find(i) != store_impl.end())
			|| (prev_scope && prev_scope->contains(i));
	}

	typedef void** stored;

	Store(){}

	Store(Store  * const prev):prev_scope(prev){}

	Store(const Store&) = delete;

	template<typename T>
	void insert(int i, const T &item) {
		assert(valid_store);
		store_impl[i].reset((stored)heap_copy(item));
		assert(contains(i));
	}

	template<typename T, typename... Args>
	void emplace(int i, Args && ... args){
		assert(valid_store);
		store_impl[i].reset((void**)new T(std::forward<Args...>(args...)));
		assert(contains(i));
	}

#define store_get_impl					  \
	if (!contains(i) && prev_scope)		  \
		return prev_scope->get<T>(i);	  \
	else if (!contains(i)) {											\
		std::cerr << "trying to find something of id " << i << std::endl; \
		assert(false && "Error: we don't have that here");				\
	}																	\
	T* ret = (T*) (store_impl.at(i).get());								\
	assert(ret);														\
	return *ret;
	
	template<typename T>
	T& get(int i){
		store_get_impl
	}

	template<typename T>
	const T& get(int i) const{
		store_get_impl
	}


	virtual ~Store() {
		valid_store = false;
		std::cout << "destroying store" << std::endl;
	}

};

