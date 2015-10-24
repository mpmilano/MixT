//What goes here: managed tracking of stores, explicitly for
//cross-store stuff.

#pragma once

#include "GDataStore.hpp"
#include "compile-time-lambda.hpp"
#include "utils.hpp"
#include <functional>

//TODO: replace with non-dummy types
template<Level l, HandleAccess HA, typename T>
struct Handle;

class Tracker {
public:
	struct TrackerDSStrong;
	struct TrackerDSCausal;
	using replicaID = const int;
	using Nonce = const int;
	using read_pair = const std::pair<replicaID, Nonce>;
	using Ends = int;
	using Metadata = long;
	using Tombstone = double;
	
	struct Internals;
private:
	Internals *i;

	TrackerDSStrong wrapStore(
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
					   std::function<Handle<Level::causal, HandleAccess::all, Metadata> (int)> existingMeta
					   
		);
	
	TrackerDSCausal wrapStore(
					   std::function<Handle<Level::causal, HandleAccess::all, Ends> (int, const Ends&)> newEnds,
					   std::function<Handle<Level::causal, HandleAccess::all, Metadata> (int, const Metadata&)> newMeta,
					   std::function<Handle<Level::causal, HandleAccess::all, Tombstone> (int, const Tombstone&)> newTomb,
					   std::function<bool (int)> exists,
					   std::function<Handle<Level::causal, HandleAccess::all, Metadata> (int)> existingMeta);
	
public:
	static Tracker& global_tracker();

	bool registered(const GDataStore&) const;

	void registerStore(DataStore<Level::strong> &, const TrackerDSStrong&, getStrongInstance);
	void registerStore(DataStore<Level::causal> &, const TrackerDSCausal&, getCausalInstance);

	template<Level l, typename NF, typename EF, typename Ex>
	auto wrapStore(const NF &nf, const Ex &ex, const EF &ef){
		return wrapStore(nf,nf,nf,ex);
	}

	template<typename DS, typename NF, typename EF, typename Ex, Ret>
	void registerStore(DS &ds, const NF &nf, const Ex &ex, Ret (*f) (replicaID)){
		registerStore(ds,wrapStore(nf,ex),f);
	}

	void tick();
	
	void onWrite(DataStore<Level::strong>&, int name);

	void onWrite(DataStore<Level::causal>&, int name);
	
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
		const std::function<Ends (std::vector<std::unique_ptr<GeneralRemoteObject>>)>&
		mergeEnds
		);
public:

	static Ends merge_ends(const std::vector<Ends> &v);

	//need to know the type of the object we are writing here.
	//thus, a lot of this work needs to get done in the header (sadly).
	template<typename T, template<class> typename RO, typename DS>
	auto onRead(DS& ds, int name,
				const std::function<std::unique_ptr<T> (std::vector<std::unique_ptr<RO<T> > >)> &merge,
				const std::function<RO<T> (int)> &existingObject,
				const std::function<Ends (
					std::vector<std::unique_ptr<RO<Ends> > >)> &mergeEnds
				= [](auto &v){return merge_ends(map(v,[](auto &a){return a->get();}));}
		){
		static_assert(std::is_base_of<DataStore<Level::causal>,DS>::value,
			"Error: first argument must be a DataStore");
		static_assert(std::is_base_of<RemoteObject<T> >,RO<T> >::value,
			"Error: RO must be *your* RemoteObject type");
		std::unique_ptr<T> ret;
		return onRead_internal(ds,name,existingObject,[&](auto &v){ret.reset(merge(v));},mergeEnds);
	}

	Tracker();
	virtual ~Tracker();

	Tracker(const Tracker&) = delete;
};
