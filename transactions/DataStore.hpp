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

		//we'll delete the TransactionContext
		//when the transaction is over.  Do any cleanup you need to do then.
		//the parameters to this function should just be passed directly to TransactionContext's constructor.
		virtual std::unique_ptr<mtl::StoreContext<Level::strong> > begin_transaction() = 0;
		DataStore(std::function<void (std::string)> logger):GDataStore{Level::strong,logger}{}
	
		DECLARED_OPERATIONS
	};

	template<>
	class DataStore<Level::causal> : public GDataStore{
	public:

		//we'll delete the TransactionContext
		//when the transaction is over.  Do any cleanup you need to do then.
		//the parameters to this function should just be passed directly to TransactionContext's constructor.
		virtual std::unique_ptr<mtl::StoreContext<Level::causal> > begin_transaction() = 0;
		DataStore(std::function<void (std::string)> logger):GDataStore{Level::causal,logger}{}

		virtual const std::array<int, NUM_CAUSAL_GROUPS>& local_time() const = 0;
	
		DECLARED_OPERATIONS

	};
}
