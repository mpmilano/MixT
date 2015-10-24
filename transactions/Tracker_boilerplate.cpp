#include "Tracker_common.hpp"
#include <map>

TrackerDSStrong Tracker::wrapStore(
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
	return  TrackerDSStrong{
		newEnds,newMeta,newTomb,exists,existingEnds,existingMeta,existingTomb};
}

TrackerDSCausal Tracker::wrapStore(
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
	return TrackerDSCausal{
		newEnds,newMeta,newTomb,exists,existingEnds,existingMeta,existingTomb};
}
