#pragma once
#include "Tracker.hpp"

template<Level l>
struct TrackerDS {
	
	DataStore<l> &real;
	Handle<l, HandleAccess::all, Tracker::Ends> (*newEnds) (DataStore<l>&, int, const Tracker::Ends&);
	Handle<l, HandleAccess::all, Tracker::Metadata> (*newMeta) (DataStore<l>&, int, const Tracker::Metadata&);
	Handle<l, HandleAccess::all, Tracker::Tombstone> (*newTomb) (DataStore<l>&, int, const Tracker::Tombstone&);
	bool (*exists) (DataStore<l>&, int);
	Handle<Level::causal, HandleAccess::all, Tracker::Metadata> (*existingMeta) (DataStore<l>&, int);

	TrackerDS(
			DataStore<l> &real,
			Handle<l, HandleAccess::all, Tracker::Ends> (*newEnds) (DataStore<l>&, int, const Tracker::Ends&),
			Handle<l, HandleAccess::all, Tracker::Metadata> (*newMeta) (DataStore<l>&, int, const Tracker::Metadata&),
			Handle<l, HandleAccess::all, Tracker::Tombstone> (*newTomb) (DataStore<l>&, int, const Tracker::Tombstone&),
			bool (*exists) (DataStore<l>&, int),
			Handle<Level::causal, HandleAccess::all, Tracker::Metadata> (*existingMeta) (DataStore<l>&, int)
		):
		real(real),
		newEnds(newEnds),
		newMeta(newMeta),
		newTomb(newTomb),
		exists(exists),
		existingMeta(existingMeta) {}

};


//I'm just going to guess the names of the functions here.
template<typename DS>
auto wrapStore(DS &ds){
	static auto newObject = [](auto &_ds, auto name, auto &e){
		auto &ds = dynamic_cast<DS&>(_ds);
		return ds.template newObject<HandleAccess::all>(name,e);
	};
	static auto exists = [](auto &_ds, auto name){
		auto &ds = dynamic_cast<DS&>(_ds);
		return ds.exists(name);
	};
	static auto existingMeta = [](auto &_ds, auto name){
		auto &ds = dynamic_cast<DS&>(_ds);
		return ds.template existingObject<HandleAccess::all,Tracker::Metadata>(name);
	};
	constexpr Level l = ds_level(mke_p<DS>());
	return TrackerDS<l>{ds,newObject, newObject, newObject,exists,existingMeta};
}

template<typename DS, typename Ret>
void Tracker::registerStore(DS &ds, Ret (*f) (Tracker::replicaID)){
	registerStore(ds,wrapStore(ds),f);
}
