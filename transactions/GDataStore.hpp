#pragma once
#include <functional>
#include "Basics.hpp"
#include "TransactionBasics.hpp"

namespace myria{


	struct GDataStore {
		virtual int ds_id() const = 0;
		virtual int instance_id() const = 0;
		virtual bool in_transaction() const = 0; //mostly for debugging
#ifndef NDEBUG
		virtual std::string why_in_transaction() const = 0; //entirely for debugging
#endif
        virtual ~GDataStore() {}
	};

	template<Level l>
	class DataStore;

}
