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

template<typename T, template<typename> class RO, typename DS>
std::unique_ptr<T> Tracker::onRead(DS& ds, int name,
						  const std::function<std::unique_ptr<T> (const std::vector<std::unique_ptr<RO<T> > >&)> &merge,
						  const std::function<std::unique_ptr<Tracker::Ends> (const std::vector<std::unique_ptr<RO<Tracker::Ends> > >&)> &mergeEnds){
	static_assert(std::is_base_of<DataStore<Level::causal>,DS>::value,
				  "Error: first argument must be a DataStore");
	static_assert(std::is_base_of<RemoteObject<T> ,RO<T> >::value,
				  "Error: RO must be *your* RemoteObject type");
	
	static auto existingEnds = [](auto &_ds, auto name){
		auto &ds = dynamic_cast<DS&>(_ds);
		return std::unique_ptr<GeneralRemoteObject>(ds.template existingRaw<Ends>(name).release());
	};
	static auto existingT = [](auto &_ds, auto name) {
		auto &ds = dynamic_cast<DS&>(_ds);
		return std::unique_ptr<GeneralRemoteObject>(ds.template existingRaw<T>(name).release());
	};
	static auto castBack = [](std::unique_ptr<GeneralRemoteObject> p) -> std::unique_ptr<RO<T> > {
		if (auto *pt = dynamic_cast<RO<T>* >(p.release()))
				return std::unique_ptr<RO<T> >(pt);
		else assert(false && "Error casting from GeneralRemoteObject back to specific RO impl");
	};
	
	std::unique_ptr<T> ret;
	onRead_internal(ds,name,existingT,
					[&](const auto &v){ret.reset(merge(map(v,castBack)));},
					existingEnds,
					[&](const auto &v){return mergeEnds(map(v,castBack));});
	return ret;
}
