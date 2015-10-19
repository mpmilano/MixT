#include "Tracker_common.hpp"
#include <map>

void Tracker::registerStore(DataStore<Level::strong>& ds,
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
	){
	strongDSmap[&ds].reset( 
		 new TrackerDSStrong{
			newEnds,newMeta,newTomb,exists,existingEnds,existingMeta,existingTomb});
}

void Tracker::registerStore(DataStore<Level::causal>& ds,
					   std::function<
					   Handle<Level::causal, HandleAccess::all, Ends>
					   (int, const Ends&)> newEnds,
					   std::function<
					   Handle<Level::causal, HandleAccess::all, Metadata>
					   (int, const Metadata&)> newMeta,
					   std::function<
					   Handle<Level::causal, HandleAccess::all, Tombstone>
					   (int, const Tombstone&)> newTomb,
					   std::function<bool (int)> exists,
					   std::function<
					   Handle<Level::causal, HandleAccess::all, Ends>
					   (int)> existingEnds,
					   std::function<
					   Handle<Level::causal, HandleAccess::all, Metadata>
					   (int)> existingMeta,
					   std::function<
					   Handle<Level::causal, HandleAccess::all, Tombstone>
					   (int)> existingTomb
	){
	causalDSmap[&ds].reset( 
		new TrackerDSCausal{
			newEnds,newMeta,newTomb,exists,existingEnds,existingMeta,existingTomb});
}
