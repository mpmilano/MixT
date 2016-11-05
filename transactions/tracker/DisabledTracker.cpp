#include "Tracker.hpp"
#include "GDataStore.hpp"
#include "DataStore.hpp"

namespace myria { namespace tracker {

		Name Tracker::Tombstone::name() const { return nonce;}
		
		struct Tracker::Internals{
			DataStore<Level::strong>* strongStore{nullptr};
			DataStore<Level::causal>* causalStore{nullptr};
		};

		void Tracker::onRead(
			TrackingContext&,
			DataStore<Level::causal>&, Name, const Clock &,
			const std::function<void (char const *)> &){}
		
		void Tracker::onRead(
			TrackingContext&,
			DataStore<Level::strong>&, Name, const Clock &,
			const std::function<void (char const *)> &){}

		bool Tracker::registered(const GDataStore& gd) const {
			if (auto* ds = dynamic_cast<DataStore<Level::strong>const * >(&gd))
				return ds == i->strongStore;
			else if (auto *ds = dynamic_cast<DataStore<Level::causal> const *  >(&gd))
				return ds == i->causalStore;
			else return false;
		}
		
		const DataStore<Level::strong>& Tracker::get_StrongStore() const {return *i->strongStore;}
		const DataStore<Level::causal>& Tracker::get_CausalStore() const {return *i->causalStore;}
		
		DataStore<Level::strong>& Tracker::get_StrongStore() {return *i->strongStore;}
		DataStore<Level::causal>& Tracker::get_CausalStore() {return *i->causalStore;}

		void Tracker::registerStore(DataStore<Level::strong> &ss,
									std::unique_ptr<TrackerDSStrong>){i->strongStore = &ss;}
		void Tracker::registerStore(DataStore<Level::causal> &cs,
									std::unique_ptr<TrackerDSCausal>){i->causalStore = &cs;}

		bool Tracker::strongRegistered() const{
			return i->strongStore;
		}
		
		bool Tracker::causalRegistered() const{
			return i->causalStore;
		}

		void Tracker::exemptItem(Name ){}

                std::unique_ptr<TrackingContext> Tracker::generateContext(std::unique_ptr<mutils::abs_StructBuilder>& l, bool){
			return std::make_unique<TrackingContext>(l,*this);
		}
		
		void Tracker::onWrite(mtl::TransactionContext&, DataStore<Level::strong>&, Name , Tombstone*){}
		void Tracker::onWrite(mtl::TransactionContext&, DataStore<Level::strong>&, Name , Clock*){}
		void Tracker::onWrite(mtl::TransactionContext&, DataStore<Level::strong>&, Name , void*){}
			

		void Tracker::onWrite(DataStore<Level::causal>&, Name , const Clock &, Tombstone*){}
		void Tracker::onWrite(DataStore<Level::causal>&, Name , const Clock &, Clock*){}
		void Tracker::onWrite(DataStore<Level::causal>&, Name , const Clock &, void*){}

		void Tracker::onCreate(DataStore<Level::causal>&, Name ,Tombstone*){}
		void Tracker::onCreate(DataStore<Level::causal>&, Name ,Clock*){}
		void Tracker::onCreate(DataStore<Level::causal>&, Name ,void*){}
			
		void Tracker::onCreate(DataStore<Level::strong>&, Name , Tombstone*){}
		void Tracker::onCreate(DataStore<Level::strong>&, Name , Clock*){}
		void Tracker::onCreate(DataStore<Level::strong>&, Name , void*){}

		void Tracker::afterRead(mtl::StoreContext<Level::strong>&, TrackingContext&, 
					   DataStore<Level::strong>&, Name , Tombstone*){}
		
		void Tracker::afterRead(mtl::StoreContext<Level::strong>&, TrackingContext&, 
					   DataStore<Level::strong>&, Name , Clock*){}
		
		void Tracker::afterRead(mtl::StoreContext<Level::strong>&, TrackingContext&, 
					   DataStore<Level::strong>&, Name , void*){}

		bool Tracker::waitForRead(TrackingContext&, DataStore<Level::causal>&, Name , const Clock& , Tombstone*){return true;}
		bool Tracker::waitForRead(TrackingContext&, DataStore<Level::causal>&, Name , const Clock& , Clock*){return true;}
		bool Tracker::waitForRead(TrackingContext&, DataStore<Level::causal>&, Name , const Clock& , void*){return true;}
		
		void Tracker::afterRead(TrackingContext&, DataStore<Level::causal>&, Name , const Clock& , const std::vector<char> &, Tombstone*){}
		void Tracker::afterRead(TrackingContext&, DataStore<Level::causal>&, Name , const Clock& , const std::vector<char> &, Clock*){}
		void Tracker::afterRead(TrackingContext&, DataStore<Level::causal>&, Name , const Clock& , const std::vector<char> &, void*){}

		void Tracker::assert_nonempty_tracking() const {}
		const CooperativeCache& Tracker::getcache() const {assert(false);struct dead_code{}; throw dead_code{};}

		Tracker::Tracker(int cache_port, CacheBehaviors  /*= CacheBehaviors::full*/)
			:i(new Internals()),cache_port(cache_port){}
		
		Tracker::~Tracker(){delete i;}

		struct TrackingContext::Internals{};		

		TrackingContext::TrackingContext(std::unique_ptr<mutils::abs_StructBuilder>& l, Tracker& t, bool):trk(t),logger(l){}
		
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
