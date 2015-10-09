#pragma once

#include "Operation.hpp"
#include "GDataStore.hpp"
#include "Tracker.hpp"

template<Level l>
class DataStore : public GDataStore{
public:

	DataStore():GDataStore{l}{}
	
	DECLARED_OPERATIONS

};
