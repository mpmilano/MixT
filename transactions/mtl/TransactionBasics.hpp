#pragma once

namespace myria {
	
	struct GDataStore;
	namespace tracker {
		struct TrackingContext;
		class Tracker;
	}
	namespace mtl {

		struct Transaction;

		struct TransactionContext {
			tracker::TrackingContext trackingContext;
		private:
			//these two are implemented in Tracker.cpp
			void commitContext ();
			void abortContext ();
			
			bool committed = false;
			virtual bool store_commit() = 0;

		public:
			TransactionContext(decltype(trackingContext) trackingContext)
				:trackingContext(trackingContext)
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
