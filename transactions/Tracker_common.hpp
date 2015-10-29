#pragma once
#include "Tracker.hpp"
#include <tuple>



//I'm just going to guess the names of the functions here.
template<typename DS>
auto wrapStore(DS &ds){
	constexpr Level l = ds_level(mke_p<DS>());
	using newEnds_t = Handle<l, HandleAccess::all, Tracker::Ends>
		(*) (DataStore<l>&, int, const Tracker::Ends&);
	
	using newMeta_t = Handle<l, HandleAccess::all, Tracker::Metadata>
		(*) (DataStore<l>&, int, const Tracker::Metadata&);
	
	using newTomb_t = Handle<l, HandleAccess::all, Tracker::Tombstone>
		(*) (DataStore<l>&, int, const Tracker::Tombstone&);
	
	using exists_t = bool (*) (DataStore<l>&, int);
	
	using existingMeta_t =
		Handle<l, HandleAccess::all, Tracker::Metadata> (*) (DataStore<l>&, int);
	
	using TrackerDS = std::tuple<DataStore<l>*, //real
								 newEnds_t,
								 newMeta_t,
								 newTomb_t,
								 exists_t,
								 existingMeta_t
								 >;
	static auto newObject = [](DataStore<l> &_ds, int name, auto &e){
		auto &ds = dynamic_cast<DS&>(_ds);
		return ds.template newObject<HandleAccess::all>(name,e);
	};
	static exists_t exists = [](DataStore<l> &_ds, int name){
		auto &ds = dynamic_cast<DS&>(_ds);
		return ds.exists(name);
	};
	static existingMeta_t existingMeta =
		[](DataStore<l> &_ds, int name) -> Handle<l, HandleAccess::all, Tracker::Metadata>{
		auto &ds = dynamic_cast<DS&>(_ds);
		return ds.template existingObject<HandleAccess::all,Tracker::Metadata>(name);
	};
	newEnds_t newEnds = newObject;
	newMeta_t newMeta = newObject;
	newTomb_t newTomb = newObject;
	
	return std::unique_ptr<TrackerDS>(
		new TrackerDS{&ds,newEnds, newMeta, newTomb,exists,existingMeta});
}

template<typename DS, typename Ret>
void Tracker::registerStore(DS &ds, Ret (*f) (Tracker::replicaID)){
	registerStore(ds,wrapStore(ds),f);
}
