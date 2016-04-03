#include "Tracker.hpp"
#include "GDataStore.hpp"
#include "DataStore.hpp"

namespace myria { namespace tracker {

		Name Tracker::Tombstone::name() const { return nonce;}
		
		struct Tracker::Internals{
			const DataStore<Level::strong>* strongStore{nullptr};
			const DataStore<Level::causal>* causalStore{nullptr};
		};

		void Tracker::onRead(
			TrackingContext&,
			DataStore<Level::causal>&, Name name, const Clock &version,
			const std::function<void (char const *)> &construct_nd_merge){}
		
		void Tracker::onRead(
			TrackingContext&,
			DataStore<Level::strong>&, Name name, const Clock &version,
			const std::function<void (char const *)> &construct_nd_merge){}

		bool Tracker::registered(const GDataStore& gd) const {
			if (auto* ds = dynamic_cast<DataStore<Level::strong>const * >(&gd))
				return ds == i->strongStore;
			else if (auto *ds = dynamic_cast<DataStore<Level::causal> const *  >(&gd))
				return ds == i->causalStore;
			else return false;
		}
		
		const DataStore<Level::strong>& Tracker::get_StrongStore() const {return *i->strongStore;}
		const DataStore<Level::causal>& Tracker::get_CausalStore() const {return *i->causalStore;}

		void Tracker::registerStore(DataStore<Level::strong> &ss,
									std::unique_ptr<TrackerDSStrong>){i->strongStore = &ss;}
		void Tracker::registerStore(DataStore<Level::causal> &cs,
									std::unique_ptr<TrackerDSCausal>){i->causalStore = &cs;}

		void Tracker::exemptItem(Name name){}

		std::unique_ptr<TrackingContext> Tracker::generateContext(bool commitOnDelete){
			return std::make_unique<TrackingContext>(*this);
		}
		
		void Tracker::onWrite(mtl::TransactionContext&, DataStore<Level::strong>&, Name name, Tombstone*){}
		void Tracker::onWrite(mtl::TransactionContext&, DataStore<Level::strong>&, Name name, Clock*){}
		void Tracker::onWrite(mtl::TransactionContext&, DataStore<Level::strong>&, Name name, void*){}
			

		void Tracker::onWrite(DataStore<Level::causal>&, Name name, const Clock &version, Tombstone*){}
		void Tracker::onWrite(DataStore<Level::causal>&, Name name, const Clock &version, Clock*){}
		void Tracker::onWrite(DataStore<Level::causal>&, Name name, const Clock &version, void*){}

		void Tracker::onCreate(DataStore<Level::causal>&, Name name,Tombstone*){}
		void Tracker::onCreate(DataStore<Level::causal>&, Name name,Clock*){}
		void Tracker::onCreate(DataStore<Level::causal>&, Name name,void*){}
			
		void Tracker::onCreate(DataStore<Level::strong>&, Name name, Tombstone*){}
		void Tracker::onCreate(DataStore<Level::strong>&, Name name, Clock*){}
		void Tracker::onCreate(DataStore<Level::strong>&, Name name, void*){}

		void Tracker::afterRead(mtl::StoreContext<Level::strong>&, TrackingContext&, 
					   DataStore<Level::strong>&, Name name, Tombstone*){}
		
		void Tracker::afterRead(mtl::StoreContext<Level::strong>&, TrackingContext&, 
					   DataStore<Level::strong>&, Name name, Clock*){}
		
		void Tracker::afterRead(mtl::StoreContext<Level::strong>&, TrackingContext&, 
					   DataStore<Level::strong>&, Name name, void*){}

		bool Tracker::waitForRead(TrackingContext&, DataStore<Level::causal>&, Name name, const Clock& version, Tombstone*){return true;}
		bool Tracker::waitForRead(TrackingContext&, DataStore<Level::causal>&, Name name, const Clock& version, Clock*){return true;}
		bool Tracker::waitForRead(TrackingContext&, DataStore<Level::causal>&, Name name, const Clock& version, void*){return true;}
		
		void Tracker::afterRead(TrackingContext&, DataStore<Level::causal>&, Name name, const Clock& version, const std::vector<char> &data, Tombstone*){}
		void Tracker::afterRead(TrackingContext&, DataStore<Level::causal>&, Name name, const Clock& version, const std::vector<char> &data, Clock*){}
		void Tracker::afterRead(TrackingContext&, DataStore<Level::causal>&, Name name, const Clock& version, const std::vector<char> &data, void*){}

		void Tracker::assert_nonempty_tracking() const {}
		const CooperativeCache& Tracker::getcache() const {assert(false);}

		Tracker::Tracker(int cache_port, CacheBehaviors behavior /*= CacheBehaviors::full*/):i(new Internals()),cache_port(cache_port){}
		Tracker::~Tracker(){delete i;}

		struct TrackingContext::Internals{};		

		TrackingContext::TrackingContext(Tracker& t, bool commitOnDelete):trk(t){}
		
		void TrackingContext::commitContext(){}
		void TrackingContext::abortContext(){}
		TrackingContext::~TrackingContext(){}

		
	}
	namespace mtl{
		void TransactionContext::commitContext(){
			trackingContext->commitContext();
		}
		void TransactionContext::abortContext(){
			trackingContext->abortContext();
		}
	}
}
