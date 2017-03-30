#pragma once
#include <functional>
#include "Basics.hpp"
#include "TransactionContext.hpp"

namespace myria{

	namespace tracker{ class Tracker; }

	struct GDataStore {
		const std::string level_description;
		virtual tracker::Tracker& tracker() = 0;
		virtual bool in_transaction() const = 0; //mostly for debugging
#ifndef NDEBUG
		virtual std::string why_in_transaction() const = 0; //entirely for debugging
#endif
		virtual ~GDataStore() = default;
		GDataStore(std::string s):level_description(s){}
	};

}
