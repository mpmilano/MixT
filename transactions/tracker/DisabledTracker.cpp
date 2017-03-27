#include "Tracker.hpp"
#include "GDataStore.hpp"
#include "DataStore.hpp"

namespace myria { namespace tracker {

		Name Tracker::Tombstone::name() const { return nonce;}
		
		struct Tracker::Internals{
			GDataStore* strongStore{nullptr};
			GDataStore* causalStore{nullptr};
		};

		void Tracker::onRead(
			TrackingContext&,
			GDataStore&, Name, const Clock &,
			const std::function<void (char const *)> &, std::true_type*){}
		
		void Tracker::onRead(
			TrackingContext&,
			GDataStore&, Name, const Clock &,
			const std::function<void (char const *)> &, std::false_type*){}

		bool Tracker::registered(const GDataStore& gd) const {
		  auto *ds = &gd;
		  return ds == i->strongStore || ds == i->causalStore;
		}
		
		const GDataStore& Tracker::get_StrongStore() const {return *i->strongStore;}
		const GDataStore& Tracker::get_CausalStore() const {return *i->causalStore;}
		
		GDataStore& Tracker::get_StrongStore() {return *i->strongStore;}
		GDataStore& Tracker::get_CausalStore() {return *i->causalStore;}

		void Tracker::registerStore(GDataStore &ss,
																std::unique_ptr<GenericTrackerDS>, std::false_type*){i->strongStore = &ss;}
		void Tracker::registerStore(GDataStore &cs,
																std::unique_ptr<GenericTrackerDS>, std::true_type*){i->causalStore = &cs;}

		bool Tracker::strongRegistered() const{
			return i->strongStore;
		}
		
		bool Tracker::causalRegistered() const{
			return i->causalStore;
		}

		void Tracker::exemptItem(Name ){}

		std::unique_ptr<TrackingContext> Tracker::generateContext(bool){
			return std::make_unique<TrackingContext>(*this);
		}
		
		void Tracker::onStrongWrite(GDataStore&, Name , Tombstone*){}
		void Tracker::onStrongWrite(GDataStore&, Name , Clock*){}
		void Tracker::onStrongWrite(GDataStore&, Name , void*){}
			

		void Tracker::onCausalWrite(GDataStore&, Name , const Clock &, Tombstone*){}
		void Tracker::onCausalWrite(GDataStore&, Name , const Clock &, Clock*){}
		void Tracker::onCausalWrite(GDataStore&, Name , const Clock &, void*){}

		void Tracker::onCausalCreate(GDataStore&, Name ,Tombstone*){}
		void Tracker::onCausalCreate(GDataStore&, Name ,Clock*){}
		void Tracker::onCausalCreate(GDataStore&, Name ,void*){}
			
		void Tracker::onStrongCreate(GDataStore&, Name , Tombstone*){}
		void Tracker::onStrongCreate(GDataStore&, Name , Clock*){}
		void Tracker::onStrongCreate(GDataStore&, Name , void*){}


    void Tracker::afterStrongRead(mtl::GStoreContext&, TrackingContext&, 
				  GDataStore&, Name , Tombstone*){}
    void Tracker::afterStrongRead(mtl::GStoreContext&, TrackingContext&, 
				  GDataStore&, Name , Clock*){}
    void Tracker::afterStrongRead(mtl::GStoreContext&, TrackingContext&, 
				  GDataStore&, Name , void*){}

    bool Tracker::waitForCausalRead(TrackingContext&, GDataStore&, Name, const Tracker::Clock& , Tracker::Tombstone*){return true;}
    bool Tracker::waitForCausalRead(TrackingContext&, GDataStore&, Name , const Tracker::Clock& , Tracker::Clock*){return true;}
    bool Tracker::waitForCausalRead(TrackingContext&, GDataStore&, Name , const Tracker::Clock& , void*){return true;}
    
    void Tracker::afterCausalRead(TrackingContext&, GDataStore&, Name , const Tracker::Clock& , const std::vector<char> &, Tracker::Tombstone*){}
    void Tracker::afterCausalRead(TrackingContext&, GDataStore&, Name , const Tracker::Clock& , const std::vector<char> &, Tracker::Clock*){}
    void Tracker::afterCausalRead(TrackingContext&, GDataStore&, Name , const Tracker::Clock& , const std::vector<char> &, void*){}

    void Tracker::assert_nonempty_tracking() const {}
    const CooperativeCache& Tracker::getcache() const {assert(false);struct dead_code{}; throw dead_code{};}
    
    Tracker::Tracker(int cache_port, CacheBehaviors  /*= CacheBehaviors::full*/)
      :i(new Internals()),cache_port(cache_port){}
    
    Tracker::~Tracker(){delete i;}
    
    struct TrackingContext::Internals{};		
    
    TrackingContext::TrackingContext(Tracker& t, bool):trk(t){}
    
    void TrackingContext::commitContext(){}
    void TrackingContext::abortContext(){}
    TrackingContext::~TrackingContext(){}
  }
}
