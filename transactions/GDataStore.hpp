#pragma once
#include "Basics.hpp"

struct GDataStore {
	const Level level;
	virtual bool in_transaction() const = 0;
	virtual void begin_transaction() = 0;
	virtual void end_transaction() = 0;

	GDataStore(Level l):level(l){}
	
};

template<Level l>
class DataStore;
