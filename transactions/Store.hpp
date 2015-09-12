#pragma once
#include "utils.hpp"

//TODO: define this better and move it.
template<bool acts_as_store>
struct StoreMap {
	
	static std::map<int, StoreMap*>& lost_and_found() {
		static std::map<int, StoreMap*> ret;
		return ret;
	};

	std::map<int,std::unique_ptr<void*> > store_impl;
	
	//you are *not* responsible for deleting this
	StoreMap * const  prev_scope = nullptr;
	bool valid_store = true;
	bool contains(int i) const{
		return (store_impl.find(i) != store_impl.end())
			|| (prev_scope && prev_scope->contains(i));
	}

	typedef void** stored;

	StoreMap(){}

	StoreMap(StoreMap  * const prev):prev_scope(prev){}

	StoreMap(const StoreMap&) = delete;

	template<typename T>
	void insert(int i, const T &item) {
		assert(valid_store);
		assert(!contains(i));
		store_impl[i].reset((stored)heap_copy(item).release());
		lost_and_found()[i] = this;
		assert(contains(i));
	}

	template<typename T, typename... Args>
	void emplace_ovrt(int i, Args && ... args){
		assert(valid_store);
		store_impl[i].reset((void**)new T(std::forward<Args>(args)...));
		lost_and_found()[i] = this;
		assert(contains(i));
	}

	template<typename T, typename... Args>
	void emplace(int i, Args && ... args){
		assert(valid_store);
		assert(!contains(i));
		emplace_ovrt<T>(i,std::forward<Args>(args)...);
	}


#define store_get_impl					  \
	if (!contains(i) && prev_scope)		  \
		return prev_scope->get<T>(i);	  \
	else if (!contains(i)) {											\
		std::cerr << "trying to find something of id " << i << std::endl; \
		std::cerr << "we have that here : "	<< lost_and_found()[i] <<std::endl; \
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


	virtual ~StoreMap() {
		valid_store = false;
		std::cout << "destroying store" << std::endl;
	}

};

using Store = StoreMap<true>;
using Cache = StoreMap<false>;
