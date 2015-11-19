#include "Tracker.hpp"

Tracker::Tracker(){}

Tracker& Tracker::global_tracker(){
		static Tracker t;
		return t;
	}

	void Tracker::registerStore(DataStore<Level::strong>&,
					   std::function<
					   Handle<Level::strong, HandleAccess::all, Ends>
					   (int, const Ends&)> newEnds,
					   std::function<
					   Handle<Level::strong, HandleAccess::all, Metadata>
					   (int, const Metadata&)> newMeta,
					   std::function<
					   Handle<Level::strong, HandleAccess::all, Tombstone>
					   (int, const Tombstone&)> newTomb,
					   std::function<bool (int)> exists,
					   std::function<
					   Handle<Level::strong, HandleAccess::all, Ends>
					   (int)> existingEnds,
					   std::function<
					   Handle<Level::strong, HandleAccess::all, Metadata>
					   (int)> existingMeta,
					   std::function<
					   Handle<Level::strong, HandleAccess::all, Tombstone>
					   (int)> existingTomb
		) {}

	
	void Tracker::registerStore(DataStore<Level::causal>&,
					   std::function<Handle<Level::causal, HandleAccess::all, Ends> (int, const Ends&)> newEnds,
					   std::function<Handle<Level::causal, HandleAccess::all, Metadata> (int, const Metadata&)> newMeta,
					   std::function<Handle<Level::causal, HandleAccess::all, Tombstone> (int, const Tombstone&)> newTomb,
					   std::function<bool (int)> exists,
					   std::function<Handle<Level::causal, HandleAccess::all, Ends> (int)> existingEnds,
					   std::function<Handle<Level::causal, HandleAccess::all, Metadata> (int)> existingMeta,
					   std::function<Handle<Level::causal, HandleAccess::all, Tombstone> (int)> existingTomb
		) {}

bool Tracker::registered(const GDataStore&) const{ return true;}

void Tracker::tick() {}

void Tracker::onWrite(DataStore<Level::causal>&, Name name){}
	
void Tracker::onWrite(DataStore<Level::strong>&, Name name){}

void Tracker::onRead(DataStore<Level::causal>&, Name name){}

void Tracker::onRead(DataStore<Level::strong>&, Name name){}

struct Tracker::TrackerDSStrong {};
struct Tracker::TrackerDSCausal {};
