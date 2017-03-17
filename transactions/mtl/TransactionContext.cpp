#include "TransactionContext.hpp"
#include "Tracker.hpp"


namespace myria {

  namespace mtl {

		GTransactionContext::GTransactionContext(tracker::Tracker& t)
	:trk(t),trackingContext(t.generateContext()){}

	}}
