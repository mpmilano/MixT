#pragma once

#include "GDataStore.hpp"
#include "Basics.hpp"

namespace myria{

	template<typename l, bool requires_tracking>
	class _DataStore;

	template<typename l>
	class _DataStore<l, false> : public GDataStore{
	public:

		//we'll delete the TransactionContext
		//when the transaction is over.  Do any cleanup you need to do then.
		//the parameters to this function should just be passed directly to TransactionContext's constructor.
		virtual std::unique_ptr<mtl::StoreContext<l> > begin_transaction(std::unique_ptr<mutils::abs_StructBuilder>&
#ifndef NDEBUG
										     , const std::string& why
#endif
										     ) = 0;
	  _DataStore(): GDataStore(){}
		virtual ~_DataStore(l::description) = default;
	
	};

	template<typename l>
	class _DataStore<l, true> : public GDataStore{
	public:

		//we'll delete the TransactionContext
		//when the transaction is over.  Do any cleanup you need to do then.
		//the parameters to this function should just be passed directly to TransactionContext's constructor.
        virtual std::unique_ptr<mtl::StoreContext<l> > begin_transaction(std::unique_ptr<mutils::abs_StructBuilder>&
#ifndef NDEBUG
										     , const std::string& why
#endif
										     ) = 0;
	  _DataStore() : GDataStore(){}

		virtual const std::array<int, NUM_CAUSAL_GROUPS>& local_time() const = 0;
		virtual ~_DataStore(l::description) = default;
	
	};

	template<typename l>
	using DataStore = _DataStore<l,l::requires_causal_tracking::value>;
}
