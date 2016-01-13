#pragma once
#include "TrackingContext.hpp"
#include "Basics.hpp"

namespace myria {
	
	struct GDataStore;
	template<Level l>
	class DataStore;

	namespace mtl {

		struct Transaction;

		template<Level l>
		struct StoreContext{
			virtual DataStore<l>& store() = 0;
			virtual bool store_commit() = 0;
		};

		struct TransactionContext {
			std::unique_ptr<tracker::TrackingContext> trackingContext;
			std::unique_ptr<StoreContext<Level::strong> > strongContext{nullptr};
			std::unique_ptr<StoreContext<Level::causal> > causalContext{nullptr};
			
		private:
			//these two are implemented in Tracker.cpp
			void commitContext ();
			void abortContext ();
			
			bool committed = false;

		public:
			TransactionContext(decltype(trackingContext) trackingContext)
				:trackingContext(std::move(trackingContext))
				{}

			TransactionContext(decltype(strongContext) sc, decltype(trackingContext) trackingContext)
				:trackingContext(std::move(trackingContext)),strongContext(std::move(sc))
				{}

			TransactionContext(decltype(causalContext) cc, decltype(trackingContext) trackingContext)
				:trackingContext(std::move(trackingContext)),causalContext(std::move(cc))
				{}
			
			bool full_commit();

			virtual ~TransactionContext(){
				if (!committed) abortContext();
			}

			friend struct Transaction;
		};
	} }
