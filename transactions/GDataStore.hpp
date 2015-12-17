#pragma once
#include "Basics.hpp"
#include "TransactionBasics.hpp"

namespace myria{


	struct GDataStore {
		const Level level;

		//we'll delete the TransactionContext
		//when the transaction is over.  Do any cleanup you need to do then.
		//the parameters to this function should just be passed directly to TransactionContext's constructor.
		virtual std::unique_ptr<mtl::TransactionContext> begin_transaction(tracker::Tracker&) = 0;
		virtual int ds_id() const = 0;
		virtual int instance_id() const = 0;

		GDataStore(Level l):level(l){}
		virtual ~GDataStore(){}
	};

	template<Level l>
	class DataStore;

	template<Level l>
	constexpr Level ds_level(DataStore<l> const * const){
		return l;
	}
}
