#pragma once
#include "Tracker.hpp"


template<Level l>
struct TrackerDS {
	DataStore<l> &real;
	const std::function<
		Handle<l, HandleAccess::all, Ends>
		(int, const Ends&)> newEnds;
	
	const std::function<
		Handle<l, HandleAccess::all, Metadata>
		(int, const Metadata&)> newMeta;
	
	const std::function<
		Handle<l, HandleAccess::all, Tombstone>
		(int, const Tombstone&)> newTomb;
	
	const std::function<bool (int)> exists;
	
	const std::function<
		Handle<l, HandleAccess::all, Ends>
		(int)> existingEnds;
	
	const std::function<
		Handle<l, HandleAccess::all, Metadata>
		(int)> existingMeta;
	
	const std::function<
		Handle<l, HandleAccess::all, Tombstone>
		(int)> existingTomb;


	TrackerDS(
		std::function<Handle<l, HandleAccess::all, Ends> (int, const Ends&)> newEnds,
		std::function<Handle<l, HandleAccess::all, Metadata> (int, const Metadata&)> newMeta,
		std::function<Handle<l, HandleAccess::all, Tombstone> (int, const Tombstone&)> newTomb,
		std::function<bool (int)> exists,
		std::function<Handle<l, HandleAccess::all, Ends> (int)> existingEnds,
		std::function<Handle<l, HandleAccess::all, Metadata> (int)> existingMeta,
		std::function<Handle<l, HandleAccess::all, Tombstone> (int)> existingTomb):
		newEnds(newEnds),
		newMeta(newMeta),
		newTomb(newTomb),
		exists(exists),
		existingEnds(existingEnds),
		existingMeta(existingMeta),
		existingTomb(existingTomb) {}

	TrackerDS(const TrackerDS&) = delete;
};

struct Tracker::TrackerDSStrong : public TrackerDS<Level::strong> {
	TrackerDSStrong(
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
		(int)> existingTomb)
	:TrackerDS<Level::strong>(newEnds, newMeta, newTomb, exists, existingEnds, existingMeta, existingTomb) {}
		
};
struct Tracker::TrackerDSCausal : public TrackerDS<Level::causal> {
	TrackerDSCausal(
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
		(int)> existingTomb)
	:TrackerDS<Level::causal>(newEnds, newMeta, newTomb, exists, existingEnds, existingMeta, existingTomb) {}	
};
