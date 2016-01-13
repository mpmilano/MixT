#pragma once
#include "TrackingContext.hpp"

namespace myria {
	
	struct GDataStore;

	namespace mtl {

		struct Transaction;

		template<Level l>
		struct StoreContext{
			virtual DataStore<l>& store() = 0;
			virtual bool store_commit() = 0;
		};

		struct TransactionContext {
			std::unique_ptr<tracker::TrackingContext> trackingContext;
			std::unique_ptr<StoreContext<Level::strong> > strongContext;
			std::unique_ptr<StoreContext<Level::causal> > causalContext;
			
		private:
			//these two are implemented in Tracker.cpp
			void commitContext ();
			void abortContext ();
			
			bool committed = false;

		public:
			TransactionContext(decltype(trackingContext) trackingContext)
				:trackingContext(std::move(trackingContext))
				{}
			
			bool full_commit(){
				auto ret = store_commit();
				if (ret){
					commitContext();
					committed = true;
				}
				return ret;
			}

			virtual ~TransactionContext(){
				if (!committed) abortContext();
			}

			friend struct Transaction;
		};
	} }
