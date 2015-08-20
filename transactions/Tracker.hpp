//What goes here: managed tracking of stores, explicitly for
//cross-store stuff.
#include "GDataStore.hpp"

#pragma once

struct Tracker {
	static Tracker global_tracker;

	void registerStore(const GDataStore&) {
		//TODO - impl
	}
};
