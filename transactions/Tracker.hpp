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
		DataStore<Level::strong> &real,
		Handle<Level::strong, HandleAccess::all, Ends> (*newEnds) (DataStore<Level::strong>&, int, const Ends&),
		Handle<Level::strong, HandleAccess::all, Metadata> (*newMeta) (DataStore<Level::strong>&, int, const Metadata&),
		Handle<Level::strong, HandleAccess::all, Tombstone> (*newTomb) (DataStore<Level::strong>&, int, const Tombstone&),
		bool (*exists) (DataStore<Level::strong>&, int),
		Handle<Level::causal, HandleAccess::all, Metadata> (*existingMeta) (DataStore<Level::strong>&, int) existingMeta
		);
	
	TrackerDSCausal wrapStore(
		DataStore<Level::causal> &real,
		Handle<Level::causal, HandleAccess::all, Ends> (*newEnds) (DataStore<Level::causal>&, int, const Ends&),
		Handle<Level::causal, HandleAccess::all, Metadata> (*newMeta) (DataStore<Level::causal>&, int, const Metadata&),
		Handle<Level::causal, HandleAccess::all, Tombstone> (*newTomb) (DataStore<Level::causal>&, int, const Tombstone&),
		bool (*exists) (DataStore<Level::causal>&, int),
		Handle<Level::causal, HandleAccess::all, Metadata> (*existingMeta) (DataStore<Level::causal>&, int) existingMeta
		);
	
public:
	static Tracker& global_tracker();

	bool registered(const GDataStore&) const;

	void registerStore(DataStore<Level::strong> &, const TrackerDSStrong&, getStrongInstance);
	void registerStore(DataStore<Level::causal> &, const TrackerDSCausal&, getCausalInstance);

	//I'm just going to guess the names of the functions here.
	template<typename DS>
	auto wrapStore(DS &ds){
		static auto newObject = [](auto &_ds, auto name, auto &e){
			auto &ds = dynamic_cast<DS&>(_ds);
			return ds.newObject<HandleAccess::all>(name,e);
		};
		static auto exists = [](auto &_ds, auto name){
			auto &ds = dynamic_cast<DS&>(_ds);
			return ds.exists(name);
		};
		static auto existingMeta = [](auto &_ds, auto name){
			auto &ds = dynamic_cast<DS&>(_ds);
			return ds.existingObject<Metadata>(name);
		};
		return wrapStore(ds,newObject, newObject, newObject,exists,existingMeta);
	}

	template<typename DS, typename NF, typename EF, typename Ex, Ret>
	void registerStore(DS &ds, const NF &nf, const Ex &ex, Ret (*f) (replicaID)){
		registerStore(ds,wrapStore(ds),f);
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
		const std::function<std::unique_ptr<GeneralRemoteObject> (
							 DataStore<Level::causal>&, int)> &
		existingEnds,
		const std::function<Ends (std::vector<std::unique_ptr<GeneralRemoteObject>>)>&
		mergeEnds
		);
public:

	template<typename Vec>
	auto default_merge(const Vec &v){
		return Vec::value_type::merge(map(v,[](auto &a){return a->get();}));
	}

	//need to know the type of the object we are writing here.
	//thus, a lot of this work needs to get done in the header (sadly).
	template<typename T, template<class> typename RO, typename DS>
	auto onRead(DS& ds, int name,
				const std::function<std::unique_ptr<T> (std::vector<std::unique_ptr<RO<T> > >)> &merge = default_merge,
				const std::function<std::unique_ptr<Ends> (std::vector<std::unique_ptr<RO<Ends> > >)> &mergeEnds = default_merge){
		static_assert(std::is_base_of<DataStore<Level::causal>,DS>::value,
			"Error: first argument must be a DataStore");
		static_assert(std::is_base_of<RemoteObject<T> >,RO<T> >::value,
			"Error: RO must be *your* RemoteObject type");
		std::unique_ptr<T> ret;
		static auto existingEnds = [](auto &_ds, auto name){
			auto &ds = dynamic_cast<DS&>(_ds);
			return std::unique_ptr<GeneralRemoteObject>(ds.existingRaw<Ends>(name).release());
		};
		static auto existingT = [](auto &_ds, auto name) -> {
			auto &ds = dynamic_cast<DS&>(_ds);
			return std::unique_ptr<GeneralRemoteObject>(ds.existingRaw<T>(name).release());
		};
		static auto castBack = [](std::unique_ptr<GeneralRemoteObject> p) -> std::unique_ptr<RO<T> > {
			if (auto *pt = dynamic_cast<RO<T>* >(p.release()))
				return std::unique_ptr<RO<T> >(pt);
			else assert(false && "Error casting from GeneralRemoteObject back to specific RO impl");
		};
		return onRead_internal(ds,name,existingT,
							   [&](const auto &v){ret.reset(merge(map(v,castBack)));},
							   existingEnds,
							   [&](const auto &v){return mergeEnds(map(v,castBack));});
	}

	Tracker();
	virtual ~Tracker();

	Tracker(const Tracker&) = delete;
};
