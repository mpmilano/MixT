//What goes here: managed tracking of stores, explicitly for
//cross-store stuff.

#pragma once

#include "CompactSet.hpp"
#include "GDataStore.hpp"
#include "compile-time-lambda.hpp"
#include "utils.hpp"
#include <functional>
#include <time.h>
#include "TrivialPair.hpp"

template<Level l, HandleAccess HA, typename T>
struct Handle;

namespace TDS{
	static constexpr int newTomb = 0;
	static constexpr int exists = 1;
	static constexpr int existingClock = 2;
}

class Tracker {
public:
	//support structures, metadata.
	using Nonce = int;
	struct Tombstone{
		Nonce nonce;
		int name() const;
	};

	using Clock = std::array<int,NUM_CAUSAL_GROUPS>;
	template<Level l>
	using TrackerDS =
		std::tuple<
		Handle<l, HandleAccess::all, Tombstone> (*)
		(DataStore<l>&, int, const Tombstone&), //newTomb
		bool (*) (DataStore<l>&, int), //exists
		std::unique_ptr<RemoteObject<l, Clock> > (*)
		(DataStore<l>&, int) //existingClock
		>;
	using TrackerDSStrong = TrackerDS<Level::strong>;
	using TrackerDSCausal = TrackerDS<Level::causal>;

	//hiding private members of this class. No implementation available.
	struct Internals;
private:
	Internals *i;

	
public:
	static Tracker& global_tracker();

	bool registered(const GDataStore&) const;

	void registerStore(DataStore<Level::strong> &, std::unique_ptr<TrackerDSStrong>);
	void registerStore(DataStore<Level::causal> &, std::unique_ptr<TrackerDSCausal>);

	template<typename DS>
	void registerStore(DS &ds);
	
	void onWrite(DataStore<Level::strong>&, int name);

	void onWrite(DataStore<Level::causal>&, int name, const Clock &version);

	void onCreate(DataStore<Level::causal>&, int name);

    void onCreate(DataStore<Level::strong>&, int name);

	void onRead(DataStore<Level::strong>&, int name);
	
	void onRead(DataStore<Level::causal>&, int name, const Clock &version);

private:
	Tracker();
	virtual ~Tracker();

	Tracker(const Tracker&) = delete;
};


