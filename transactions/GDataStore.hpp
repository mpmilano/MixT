#pragma once
#include <functional>
#include "Basics.hpp"
#include "mtl/TransactionContext.hpp"

namespace myria{

	struct GDataStore {
#ifndef NDEBUG
		virtual std::string why_in_transaction() const = 0; //entirely for debugging
#endif
		virtual ~GDataStore() = default;
	};

}
