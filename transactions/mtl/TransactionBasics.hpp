#pragma once
#include "TrackingContext.hpp"

namespace myria {
	
	struct GDataStore;

	namespace mtl {

		struct Transaction;

		struct TransactionContext {
			std::unique_ptr<tracker::TrackingContext> trackingContext;
		private:
			//these two are implemented in Tracker.cpp
			void commitContext ();
			void abortContext ();
			
			bool committed = false;
			virtual bool store_commit() = 0;

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
			virtual GDataStore& store() = 0;
			virtual ~TransactionContext(){
				if (!committed) abortContext();
			}

			friend struct Transaction;
		};
	} }
