#pragma once
#include "Tracker.hpp"
#include <tuple>

namespace myria { namespace tracker { 



		//I'm just going to guess the names of the functions here.
		template<typename DS>
		auto wrapStore(DS &ds){
			constexpr Level l = ds_level(mutils::mke_p<DS>());
			using newTomb_t = Handle<l, HandleAccess::all, Tracker::Tombstone>
				(*) (DataStore<l>&, Name, const Tracker::Tombstone&);
			using existingClock_t = std::unique_ptr<RemoteObject<l, Tracker::Clock> >
				(*) (DataStore<l>&, Name);
			using existingTomb_t = std::unique_ptr<RemoteObject<l, Tracker::Tombstone> >
				(*) (DataStore<l>&, Name);
	
			using exists_t = bool (*) (DataStore<l>&, Name);
	
			using TrackerDS = std::tuple<newTomb_t,
										 exists_t,
										 existingClock_t,
										 existingTomb_t
										 >;
			static newTomb_t newTomb = [](DataStore<l> &_ds, Name name, auto &e){
				auto &ds = dynamic_cast<DS&>(_ds);
				return ds.template newObject<HandleAccess::all>(name,e);
			};
			static exists_t exists = [](DataStore<l> &_ds, Name name){
				auto &ds = dynamic_cast<DS&>(_ds);
				return ds.exists(name);
			};
			static existingClock_t existClock =
				[](DataStore<l> &_ds, Name name) -> std::unique_ptr<RemoteObject<l, Tracker::Clock> >{
				auto &ds = dynamic_cast<DS&>(_ds);
				return ds.template existingRaw<Tracker::Clock>(name);
			};
			static existingTomb_t existTomb =
				[](DataStore<l> &_ds, Name name) -> std::unique_ptr<RemoteObject<l, Tracker::Tombstone> >{
				auto &ds = dynamic_cast<DS&>(_ds);
				return ds.template existingRaw<Tracker::Tombstone>(name);
			};
			return std::unique_ptr<TrackerDS>(
				new TrackerDS{newTomb, exists,existClock,existTomb});
		}

		template<typename DS>
		void Tracker::registerStore(DS &ds){
			registerStore(ds,wrapStore(ds));
		}

	}}
