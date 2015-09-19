#pragma once
#include "utils.hpp"

struct StoreMiss {};

enum class StoreType{
	StrongStore,
		CausalStore,
		StrongCache,
		CausalCache
		};

constexpr bool is_store(StoreType st){
	return st == StoreType::CausalStore || st == StoreType::StrongStore;
}

//TODO: define this better and move it.
template<StoreType semantic_switch>
struct StoreMap {
private:
	template<StoreType ss,restrict(ss != semantic_switch)>
	explicit StoreMap(StoreMap<ss> &&sm)
		:store_impl(std::move(sm.store_impl)),
		 valid_store(sm.valid_store){
		sm.valid_store = false;
	}
	
public:
	
	static std::map<int, StoreMap*>& lost_and_found() {
		static std::map<int, StoreMap*> ret;
		return ret;
	};

	bool looping = false;
	void in_loop() {
		assert(is_store(semantic_switch));
		//yeah, no nested while-loops for now.
		//will be obvious later.
		assert(!looping);
		looping = true;
	}
	void out_of_loop() {
		assert(looping);
		assert(is_store(semantic_switch));
		looping = false;
	}

	std::map<int,std::unique_ptr<void*> > store_impl;
	
	bool valid_store = true;
	bool contains(int i) const{
		return (store_impl.find(i) != store_impl.end());
	}

	typedef void** stored;

	StoreMap(){}

	StoreMap(const StoreMap&) = delete;

#define dbg_store_prnt(y,x) if (is_store(semantic_switch)) std::cout << y << " this value (" << i << "): " << x << std::endl;
#define dbg_store_prnt2 if (is_store(semantic_switch)) std::cout << "Getting this value (" << i << "): " << *ret << std::endl;
	
	template<typename T>
	void insert(int i, const T &item) {
		assert(valid_store);
		if (!looping) assert(!contains(i));
		store_impl[i].reset((stored)heap_copy(item).release());
		lost_and_found()[i] = this;
		assert(contains(i));
		dbg_store_prnt("Storing",item)
	}

	template<typename T, typename... Args>
	void emplace_ovrt(int i, Args && ... args){
		assert(valid_store);
		store_impl[i].reset((void**)new T(std::forward<Args>(args)...));
		lost_and_found()[i] = this;
		assert(contains(i));
		dbg_store_prnt("Emplacing",*((T*) store_impl.at(i).get()))
	}

	template<typename T, typename... Args>
	void emplace(int i, Args && ... args){
		assert(valid_store);
		if (!looping) assert(!contains(i));
		emplace_ovrt<T>(i,std::forward<Args>(args)...);
	}


#define store_get_impl													\
	if (!contains(i)) {													\
		std::cerr << "trying to find something of id " << i << " in a " << (is_store(semantic_switch) ? "store" : "cache" ) <<std::endl; \
		std::cerr << "we have that here : "	<< lost_and_found()[i] <<std::endl; \
		throw StoreMiss{};												\
	}																	\
	T* ret = (T*) (store_impl.at(i).get());								\
	assert(ret);														\
	dbg_store_prnt2														\
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

	friend struct Transaction;
};

/*
template<StoreType semantic_switch>
struct ROStore {
	
	const std::map<int,std::unique_ptr<void*> > &store_impl;	
	const std::function<bool (int)> contains;	

	ROStore(const StoreMap<semantic_switch> &sm):
		store_impl(sm.store_impl),
		contains([&](int j){return sm.contains(j);}){}

	template<typename T>
	T& get(int i){
		T* ret = (T*) (store_impl.at(i).get());
		dbg_store_prnt2
		return *ret;
	}
	
	template<typename T>
	const T& get(int i) const{
		T* ret = (T*) (store_impl.at(i).get());
		dbg_store_prnt2
		return *ret;
	}

	
}; //*/

using StrongStore = StoreMap<StoreType::StrongStore>;
using StrongCache = StoreMap<StoreType::StrongCache>;
using CausalStore = StoreMap<StoreType::CausalStore>;
using CausalCache = StoreMap<StoreType::CausalCache>;

template<typename T>
struct is_Cache :
	std::integral_constant<
	bool,
	std::is_same<StrongCache, std::decay_t<T> >::value ||
	std::is_same<CausalCache, std::decay_t<T> >::value >::type
{};
