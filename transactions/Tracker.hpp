//What goes here: managed tracking of stores, explicitly for
//cross-store stuff.
#include "GDataStore.hpp"

#pragma once

struct Tracker {
	static Tracker& global_tracker(){
		static Tracker t;
		return t;
	}

	void registerStore(const GDataStore&) {
		//TODO - impl
	}




//Transaction stuff

	DataStore<Level::strong> *strongTransStore = nullptr;
	DataStore<Level::causal> *causalTransStore = nullptr;
	
	void markInTransaction(DataStore<Level::strong>& ds);

	void markInTransaction(DataStore<Level::causal>& ds);
	
	DataStore<Level::strong>* strongStoreInTransaction();

	DataStore<Level::causal>* causalStoreInTransaction();

	Tracker(){}

	Tracker(const Tracker&) = delete;
};
