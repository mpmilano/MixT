//What goes here: managed tracking of stores, explicitly for
//cross-store stuff.

#pragma once

#include "GDataStore.hpp"
#include "compile-time-lambda.hpp"
#include "utils.hpp"
#include <functional>

//TODO: replace with non-dummy types
using Ends = int;
using Metadata = long;
using Tombstone = double;

template<Level l, HandleAccess HA, typename T>
struct Handle;

class Tracker {
private:

	void registerStore(DataStore<Level::strong>&,
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
		);
	
	void registerStore(DataStore<Level::causal>&,
					   std::function<Handle<Level::causal, HandleAccess::all, Ends> (int, const Ends&)> newEnds,
					   std::function<Handle<Level::causal, HandleAccess::all, Metadata> (int, const Metadata&)> newMeta,
					   std::function<Handle<Level::causal, HandleAccess::all, Tombstone> (int, const Tombstone&)> newTomb,
					   std::function<bool (int)> exists,
					   std::function<Handle<Level::causal, HandleAccess::all, Ends> (int)> existingEnds,
					   std::function<Handle<Level::causal, HandleAccess::all, Metadata> (int)> existingMeta,
					   std::function<Handle<Level::causal, HandleAccess::all, Tombstone> (int)> existingTomb
		);
	
public:
	static Tracker& global_tracker();

	bool registered(const GDataStore&) const;

	template<typename DS, typename NF, typename EF, typename Ex>
	void registerStore(DS &ds, const NF &nf, const Ex &ex, const EF &ef){
		registerStore(ds,nf,nf,nf,ex,
					  [ef](const auto &a) {return ef(a,mke_p<Ends>()); },
					  [ef](const auto &a) {return ef(a,mke_p<Metadata>()); },
					  [ef](const auto &a) {return ef(a,mke_p<Tombstone>()); });
	}

	void tick();

	void onWrite(DataStore<Level::causal>&, int name);
	
	void onWrite(DataStore<Level::strong>&, int name);

	void onRead(DataStore<Level::causal>&, int name);
	
	void onRead(DataStore<Level::strong>&, int name);

	Tracker();

	Tracker(const Tracker&) = delete;
};
