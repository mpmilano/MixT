#pragma once
#include "Tracker.hpp"

template<Level l>
struct TrackerDS {
	
	DataStore<l> &real;
	Handle<l, HandleAccess::all, Tracker::Ends> (*newEnds) (DataStore<l>&, int, const Tracker::Ends&);
	Handle<l, HandleAccess::all, Tracker::Metadata> (*newMeta) (DataStore<l>&, int, const Tracker::Metadata&);
	Handle<l, HandleAccess::all, Tracker::Tombstone> (*newTomb) (DataStore<l>&, int, const Tracker::Tombstone&);
	bool (*exists) (DataStore<l>&, int);
	Handle<Level::causal, HandleAccess::all, Tracker::Metadata> (*existingMeta) (DataStore<l>&, int);

	TrackerDS(
			DataStore<l> &real,
			Handle<l, HandleAccess::all, Tracker::Ends> (*newEnds) (DataStore<l>&, int, const Tracker::Ends&),
			Handle<l, HandleAccess::all, Tracker::Metadata> (*newMeta) (DataStore<l>&, int, const Tracker::Metadata&),
			Handle<l, HandleAccess::all, Tracker::Tombstone> (*newTomb) (DataStore<l>&, int, const Tracker::Tombstone&),
			bool (*exists) (DataStore<l>&, int),
			Handle<Level::causal, HandleAccess::all, Tracker::Metadata> (*existingMeta) (DataStore<l>&, int)
		):
		real(real),
		newEnds(newEnds),
		newMeta(newMeta),
		newTomb(newTomb),
		exists(exists),
		existingMeta(existingMeta) {}

};

struct Tracker::TrackerDSStrong : public TrackerDS<Level::strong> {
	TrackerDSStrong(DataStore<Level::strong> &real,
					Handle<Level::strong, HandleAccess::all, Tracker::Ends> (*newEnds) (DataStore<Level::strong>&, int, const Tracker::Ends&),
					Handle<Level::strong, HandleAccess::all, Tracker::Metadata> (*newMeta) (DataStore<Level::strong>&, int, const Tracker::Metadata&),
					Handle<Level::strong, HandleAccess::all, Tracker::Tombstone> (*newTomb) (DataStore<Level::strong>&, int, const Tracker::Tombstone&),
					bool (*exists) (DataStore<Level::strong>&, int),
					Handle<Level::causal, HandleAccess::all, Tracker::Metadata> (*existingMeta) (DataStore<Level::strong>&, int)
)
	:TrackerDS<Level::strong>(real,newEnds, newMeta, newTomb, exists, existingMeta) {}
		
};

struct Tracker::TrackerDSCausal : public TrackerDS<Level::causal> {
	TrackerDSCausal(DataStore<Level::causal> &real,
					Handle<Level::causal, HandleAccess::all, Tracker::Ends> (*newEnds) (DataStore<Level::causal>&, int, const Tracker::Ends&),
					Handle<Level::causal, HandleAccess::all, Tracker::Metadata> (*newMeta) (DataStore<Level::causal>&, int, const Tracker::Metadata&),
					Handle<Level::causal, HandleAccess::all, Tracker::Tombstone> (*newTomb) (DataStore<Level::causal>&, int, const Tracker::Tombstone&),
					bool (*exists) (DataStore<Level::causal>&, int),
					Handle<Level::causal, HandleAccess::all, Tracker::Metadata> (*existingMeta) (DataStore<Level::causal>&, int)
)
	:TrackerDS<Level::causal>(real,newEnds, newMeta, newTomb, exists, existingMeta) {}
		
};
