#pragma once

#include "Operation.hpp"
#include "GDataStore.hpp"
#include "Basics.hpp"

namespace myria{

template<Level l>
class DataStore;

template<>
class DataStore<Level::strong> : public GDataStore{
public:

	DataStore():GDataStore{Level::strong}{}
	
	DECLARED_OPERATIONS
};

template<>
class DataStore<Level::causal> : public GDataStore{
public:

	DataStore():GDataStore{Level::causal}{}

	virtual const std::array<int, NUM_CAUSAL_GROUPS>& local_time() const = 0;
	
	DECLARED_OPERATIONS

};
}
