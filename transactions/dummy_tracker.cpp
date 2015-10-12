#include "Tracker.hpp"

Tracker::Tracker(){}

Tracker& Tracker::global_tracker(){
		static Tracker t;
		return t;
	}

	void Tracker::registerStore(DataStore<Level::strong>&,
					   std::function<
					   Handle<Level::strong, HandleAccess::all, Ends>
					   (const std::string&, const Ends&)> newEnds,
					   std::function<
					   Handle<Level::strong, HandleAccess::all, Metadata>
					   (const std::string&, const Metadata&)> newMeta,
					   std::function<
					   Handle<Level::strong, HandleAccess::all, Tombstone>
					   (const std::string&, const Tombstone&)> newTomb,
					   std::function<bool (const std::string&)> exists,
					   std::function<
					   Handle<Level::strong, HandleAccess::all, Ends>
					   (const std::string&)> existingEnds,
					   std::function<
					   Handle<Level::strong, HandleAccess::all, Metadata>
					   (const std::string&)> existingMeta,
					   std::function<
					   Handle<Level::strong, HandleAccess::all, Tombstone>
					   (const std::string&)> existingTomb
		) {}

	
	void Tracker::registerStore(DataStore<Level::causal>&,
					   std::function<Handle<Level::causal, HandleAccess::all, Ends> (const std::string&, const Ends&)> newEnds,
					   std::function<Handle<Level::causal, HandleAccess::all, Metadata> (const std::string&, const Metadata&)> newMeta,
					   std::function<Handle<Level::causal, HandleAccess::all, Tombstone> (const std::string&, const Tombstone&)> newTomb,
					   std::function<bool (const std::string&)> exists,
					   std::function<Handle<Level::causal, HandleAccess::all, Ends> (const std::string&)> existingEnds,
					   std::function<Handle<Level::causal, HandleAccess::all, Metadata> (const std::string&)> existingMeta,
					   std::function<Handle<Level::causal, HandleAccess::all, Tombstone> (const std::string&)> existingTomb
		) {}
