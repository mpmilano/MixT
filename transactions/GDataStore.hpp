#pragma once
#include "Basics.hpp"
#include "TransactionBasics.hpp"

struct GDataStore {
	const Level level;

	//we'll delete the TransactionContext
	//when the transaction is over.  Do any cleanup you need to do then.
	virtual std::unique_ptr<TransactionContext> begin_transaction() = 0;
	virtual int ds_id() const = 0;
	virtual int instance_id() const = 0;

	GDataStore(Level l):level(l){}
	
};

template<Level l>
class DataStore;

template<typename>
struct get_level;

template<Level l>
struct get_level<DataStore<l> > : std::integral_constant<Level,l> {};
