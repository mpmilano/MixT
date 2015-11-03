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
	static constexpr int real = 0;
	static constexpr int newEnds = 1;
	static constexpr int newMeta = 2;
	static constexpr int newTomb = 3;
	static constexpr int exists = 4;
	static constexpr int existingMeta = 5;
}

class Tracker {
public:
	//support structures, metadata.
	//implementations are in Tracker_support_structures.hpp
	struct Ends;
	struct Metadata;
	struct Tombstone;
	struct timestamp;
	struct timestamp_c;
	template<Level l>
	using TrackerDS = std::tuple<DataStore<l>*, //real
								 Handle<l, HandleAccess::all, Ends> (*) (DataStore<l>&, int, const Ends&), //newEnds
								 Handle<l, HandleAccess::all, Metadata> (*) (DataStore<l>&, int, const Metadata&), //newMeta
								 Handle<l, HandleAccess::all, Tombstone> (*) (DataStore<l>&, int, const Tombstone&), //newTomb
								 bool (*) (DataStore<l>&, int), //exists
								 Handle<l, HandleAccess::all, Metadata> (*) (DataStore<l>&, int) //existingMeta
								 >;
	using TrackerDSStrong = TrackerDS<Level::strong>;
	using TrackerDSCausal = TrackerDS<Level::causal>;
	using replicaID = int;
	using Nonce = int;
	using read_pair = TrivialPair<replicaID, Nonce>;

	typedef std::unique_ptr<TrackerDSStrong > (*getStrongInstance) (replicaID);
	typedef std::unique_ptr<TrackerDSCausal > (*getCausalInstance) (replicaID);

	//hiding private members of this class. No implementation available.
	struct Internals;
private:
	Internals *i;

	
public:
	static Tracker& global_tracker();

	bool registered(const GDataStore&) const;

	void registerStore(DataStore<Level::strong> &, std::unique_ptr<TrackerDSStrong>, getStrongInstance);
	void registerStore(DataStore<Level::causal> &, std::unique_ptr<TrackerDSCausal>, getCausalInstance);

	template<typename DS, typename Ret>
	void registerStore(DS &ds, Ret (*f) (replicaID));

	void tick();
	
	void onWrite(DataStore<Level::strong>&, int name);

	void onWrite(DataStore<Level::causal>&, int name);

	void onCreate(DataStore<Level::causal>&, int name);
	
	void onRead(DataStore<Level::strong>&, int name);

private:
	void onRead_internal(
		DataStore<Level::causal>&,
		int name,
		const std::function<std::unique_ptr<GeneralRemoteObject> (
			DataStore<Level::causal>&, int)> &
		existingT,
		const std::function<void (std::vector<std::unique_ptr<GeneralRemoteObject>>)>&
		mergeT,
		const std::function<std::unique_ptr<GeneralRemoteObject> (
							 DataStore<Level::causal>&, int)> &
		existingEnds,
		const std::function<std::unique_ptr<Ends> (std::vector<std::unique_ptr<GeneralRemoteObject>>)>&
		mergeEnds
		);
public:

	template<template<typename> class RO, typename T>
	static auto default_merge(const std::vector<std::unique_ptr<RO<T> > > &v)  {
		std::vector<T const * > arg = map(v,[](const std::unique_ptr<RO<T> > &a) -> T const *  {return &a->get(nullptr);});
		return T::merge(arg);
	}

	//need to know the type of the object we are writing here.
	//thus, a lot of this work needs to get done in the header (sadly).
	template<typename T, template<typename> class RO, typename DS>
	std::unique_ptr<T> onRead(DS& ds, int name,
							  const std::function<std::unique_ptr<T> (const std::vector<std::unique_ptr<RO<T> > >&)> &merge
							  = default_merge<RO,T>,
							  const std::function<std::unique_ptr<Ends> (const std::vector<std::unique_ptr<RO<Ends> > >&)> &mergeEnds =
							  default_merge<RO,Ends>);

	Tracker();
	virtual ~Tracker();

	Tracker(const Tracker&) = delete;
};


