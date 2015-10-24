#include "Tracker_common.hpp"
#include <map>

TrackerDSStrong Tracker::wrapStore(
		DataStore<Level::strong> &real,
		Handle<Level::strong, HandleAccess::all, Ends> (*newEnds) (DataStore<Level::strong>&, int, const Ends&),
		Handle<Level::strong, HandleAccess::all, Metadata> (*newMeta) (DataStore<Level::strong>&, int, const Metadata&),
		Handle<Level::strong, HandleAccess::all, Tombstone> (*newTomb) (DataStore<Level::strong>&, int, const Tombstone&),
		bool (*exists) (DataStore<Level::strong>&, int),
		Handle<Level::causal, HandleAccess::all, Metadata> (*existingMeta) (DataStore<Level::strong>&, int) existingMeta,
		Handle<Level::causal, HandleAccess::all, Ends> (*existingEnds) (DataStore<Level::strong>&, int) existingEnds
		){
	return  TrackerDSStrong{
		real,newEnds,newMeta,newTomb,exists,existingMeta,existingEnds};
}

TrackerDSCausal Tracker::wrapStore(
		DataStore<Level::causal> &real,
		Handle<Level::causal, HandleAccess::all, Ends> (*newEnds) (DataStore<Level::causal>&, int, const Ends&),
		Handle<Level::causal, HandleAccess::all, Metadata> (*newMeta) (DataStore<Level::causal>&, int, const Metadata&),
		Handle<Level::causal, HandleAccess::all, Tombstone> (*newTomb) (DataStore<Level::causal>&, int, const Tombstone&),
		bool (*exists) (DataStore<Level::causal>&, int),
		Handle<Level::causal, HandleAccess::all, Metadata> (*existingMeta) (DataStore<Level::causal>&, int) existingMeta,
		Handle<Level::causal, HandleAccess::all, Ends> (*existingEnds) (DataStore<Level::causal>&, int) existingEnds
		){
	return  TrackerDSCausal{
		real,newEnds,newMeta,newTomb,exists,existingMeta,existingEnds};
}
