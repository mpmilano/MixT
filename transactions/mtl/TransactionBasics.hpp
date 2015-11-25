#pragma once

namespace myria { namespace mtl {

struct GDataStore;

struct TransactionContext {

	virtual bool commit() = 0;
	virtual GDataStore& store() = 0 ;
	virtual ~TransactionContext(){}
	
};


	} }
