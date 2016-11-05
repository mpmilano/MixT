#pragma once
#include <functional>
#include "Basics.hpp"
#include "TransactionBasics.hpp"

namespace myria{


	struct GDataStore {
		const Level level;
		virtual int ds_id() const = 0;
		virtual int instance_id() const = 0;
		virtual bool in_transaction() const = 0; //mostly for debugging
#ifndef NDEBUG
		virtual std::string why_in_transaction() const = 0; //entirely for debugging
#endif

		GDataStore(Level l):level(l){}
        virtual ~GDataStore() {}
	};

	template<Level l>
	class DataStore;

	template<Level l>
	constexpr Level ds_level(DataStore<l> const * const){
		return l;
	}
}
