#pragma once
#include "Tracker.hpp"

template<Level l>
struct TrackerDS {
	
	DataStore<Level::strong> &real;
	Handle<Level::strong, HandleAccess::all, Ends> (*newEnds) (DataStore<Level::strong>&, int, const Ends&);
	Handle<Level::strong, HandleAccess::all, Metadata> (*newMeta) (DataStore<Level::strong>&, int, const Metadata&);
	Handle<Level::strong, HandleAccess::all, Tombstone> (*newTomb) (DataStore<Level::strong>&, int, const Tombstone&);
	bool (*exists) (DataStore<Level::strong>&, int);
	Handle<Level::causal, HandleAccess::all, Metadata> (*existingMeta) (DataStore<Level::strong>&, int) existingMeta;

	TrackerDS(
			DataStore<Level::strong> &real,
			Handle<Level::strong, HandleAccess::all, Ends> (*newEnds) (DataStore<Level::strong>&, int, const Ends&),
			Handle<Level::strong, HandleAccess::all, Metadata> (*newMeta) (DataStore<Level::strong>&, int, const Metadata&),
			Handle<Level::strong, HandleAccess::all, Tombstone> (*newTomb) (DataStore<Level::strong>&, int, const Tombstone&),
			bool (*exists) (DataStore<Level::strong>&, int),
			Handle<Level::causal, HandleAccess::all, Metadata> (*existingMeta) (DataStore<Level::strong>&, int) existingMeta
		):
		real(real),
		newEnds(newEnds),
		newMeta(newMeta),
		newTomb(newTomb),
		exists(exists),
		existingMeta(existingMeta) {}

	TrackerDS(const TrackerDS&) = delete;
};

struct Tracker::TrackerDSStrong : public TrackerDS<Level::strong> {
	TrackerDSStrong(DataStore<Level::strong> &real,
					Handle<Level::strong, HandleAccess::all, Ends> (*newEnds) (DataStore<Level::strong>&, int, const Ends&),
					Handle<Level::strong, HandleAccess::all, Metadata> (*newMeta) (DataStore<Level::strong>&, int, const Metadata&),
					Handle<Level::strong, HandleAccess::all, Tombstone> (*newTomb) (DataStore<Level::strong>&, int, const Tombstone&),
					bool (*exists) (DataStore<Level::strong>&, int),
					Handle<Level::causal, HandleAccess::all, Metadata> (*existingMeta) (DataStore<Level::strong>&, int) existingMeta
)
	:TrackerDS<Level::strong>(real,newEnds, newMeta, newTomb, exists, existingMeta) {}
		
};

struct Tracker::TrackerDSCausal : public TrackerDS<Level::causal> {
	TrackerDSCausal(DataStore<Level::causal> &real,
					Handle<Level::causal, HandleAccess::all, Ends> (*newEnds) (DataStore<Level::causal>&, int, const Ends&),
					Handle<Level::causal, HandleAccess::all, Metadata> (*newMeta) (DataStore<Level::causal>&, int, const Metadata&),
					Handle<Level::causal, HandleAccess::all, Tombstone> (*newTomb) (DataStore<Level::causal>&, int, const Tombstone&),
					bool (*exists) (DataStore<Level::causal>&, int),
					Handle<Level::causal, HandleAccess::all, Metadata> (*existingMeta) (DataStore<Level::causal>&, int) existingMeta
)
	:TrackerDS<Level::causal>(real,newEnds, newMeta, newTomb, exists, existingMeta) {}
		
};
