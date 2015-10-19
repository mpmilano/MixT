#include "Tracker_common.hpp"
#include "DataStore.hpp"

Tracker::Tracker(){}

Tracker& Tracker::global_tracker(){
	static Tracker t;
	return t;
}

bool Tracker::registered(const GDataStore& gds) const{
	//wow it's stupid that I need this.  Basically an immutable reference to a 
	//mutable pointer is what count() takes, which isn't allowed. 
	if (auto *t1 = dynamic_cast<const DataStore<Level::strong>* >(&gds)){
		return strongDSmap.count(const_cast<DataStore<Level::strong>*>(t1)) > 0;
	}
	else if (auto *t2 = dynamic_cast<const DataStore<Level::causal>*  >(&gds)){
		return causalDSmap.count(const_cast<DataStore<Level::causal>* > (t2)) > 0;
	}
	else assert(false && "there's a third kind of GDataStore?");
}

