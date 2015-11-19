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
#include "RemoteObject.hpp"

template<Level l, HandleAccess HA, typename T>
struct Handle;

namespace TDS{
	static constexpr int newTomb = 0;
	static constexpr int exists = 1;
	static constexpr int existingClock = 2;
	static constexpr int existingTomb = 3;
}

class Tracker {
public:
	//support structures, metadata.
	using Nonce = int;
	struct Tombstone{
		Nonce nonce;
		Name name() const;
	};

	using Clock = std::array<int,NUM_CAUSAL_GROUPS>;
	template<Level l>
	using TrackerDS =
		std::tuple<
		Handle<l, HandleAccess::all, Tombstone> (*)
		(DataStore<l>&, Name, const Tombstone&), //newTomb
		bool (*) (DataStore<l>&, Name), //exists
		std::unique_ptr<RemoteObject<l, Clock> > (*)
		(DataStore<l>&, Name), //existingClock
		std::unique_ptr<RemoteObject<l, Tombstone> > (*)
		(DataStore<l>&, Name) //existingTomb
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

    void exemptItem(Name name);

    template<typename T, Level l, HandleAccess ha>
    void exemptItem(const Handle<l,ha,T>& h){
        exemptItem(h.name());
    }
	
	void onWrite(DataStore<Level::strong>&, Name name);

	void onWrite(DataStore<Level::causal>&, Name name, const Clock &version);

	void onCreate(DataStore<Level::causal>&, Name name);

    void onCreate(DataStore<Level::strong>&, Name name);

	void onRead(DataStore<Level::strong>&, Name name);
	
	void onRead(DataStore<Level::causal>&, Name name, const Clock &version, std::vector<char> bytes);

	//for testing
	void assert_nonempty_tracking() const;

private:
	Tracker();
	virtual ~Tracker();

	Tracker(const Tracker&) = delete;
};


