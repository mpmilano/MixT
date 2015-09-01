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

	Tracker(){}

	Tracker(const Tracker&) = delete;
};
