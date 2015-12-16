#pragma once

namespace myria {
	
	struct GDataStore;
	namespace tracker {
		struct TrackingContext;
		class Tracker;
	}
	namespace mtl {

		struct TransactionContext {

			tracker::TrackingContext& trackingContext;
			void (*commitContext) (tracker::TrackingContext&);
			void (*abortContext) (tracker::TrackingContext&);
			bool committed = false;
			TransactionContext(decltype(trackingContext) trackingContext,
							   decltype(commitContext) commitContext,
							   decltype(abortContext) abortContext)
				:trk(trk),trackingContext(trackingContext),commitContext(commitContext),abortContext(abortContext)
				{}
			
			bool full_commit(){
				auto ret = store_commit();
				if (ret){
					commitContext(trackingContext);
					committed = true;
				}
				return ret;
			}
			virtual bool store_commit() = 0;
			virtual GDataStore& store() = 0;
			virtual ~TransactionContext(){
				(if !committed) abortContext(trackingContext);
			}
		};
	} }
