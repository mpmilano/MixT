#pragma once
#include <functional>
#include "Basics.hpp"
#include "TransactionBasics.hpp"

namespace myria{


	struct GDataStore {
		const Level level;
		::mutils::ReassignableReference<::mutils::abs_StructBuilder> logger;
		virtual int ds_id() const = 0;
		virtual int instance_id() const = 0;
		virtual bool in_transaction() const = 0; //mostly for debugging

		GDataStore(Level l, ::mutils::ReassignableReference<::mutils::abs_StructBuilder> logger):level(l),logger(logger){}
		virtual ~GDataStore(){}
	};

	template<Level l>
	class DataStore;

	template<Level l>
	constexpr Level ds_level(DataStore<l> const * const){
		return l;
	}
}
