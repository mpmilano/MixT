#pragma once
#include "Tracker.hpp"
#include <tuple>

namespace myria { namespace tracker { 



		//I'm just going to guess the names of the functions here.
		template<typename DS>
		auto wrapStore(DS &ds){
			constexpr Level l = ds_level(mutils::mke_p<DS>());
			using newTomb_t = Handle<l, HandleAccess::all, Tracker::Tombstone>
				(*) (tracker::Tracker &trk, mtl::TransactionContext& ctx, DataStore<l>&, Name, const Tracker::Tombstone&);
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
			static const newTomb_t newTomb = [](tracker::Tracker &trk, mtl::TransactionContext& ctx, DataStore<l> &_ds, Name name, auto &e){
				auto &ds = dynamic_cast<DS&>(_ds);
				return ds.template newObject<HandleAccess::all>(trk,&ctx, name,e);
			};
			static const exists_t exists = [](DataStore<l> &_ds, Name name){
				auto &ds = dynamic_cast<DS&>(_ds);
				return ds.exists(name);
			};
			static const existingClock_t existClock =
				[](DataStore<l> &_ds, Name name) -> std::unique_ptr<RemoteObject<l, Tracker::Clock> >{
				auto &ds = dynamic_cast<DS&>(_ds);
				return ds.template existingRaw<Tracker::Clock>(name);
			};
			static const existingTomb_t existTomb =
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

		template<typename DS, typename T>
		std::unique_ptr<T>
		Tracker::onRead(TrackingContext&, DS&, Name, const Clock&, std::unique_ptr<T> candidate,
						Tombstone*, std::unique_ptr<T> (*) (char const*, std::unique_ptr<T>)){
			return candidate;
		}
		template<typename DS, typename T>
		std::unique_ptr<T>
		Tracker::onRead(TrackingContext&, DS&, Name, const Clock&, std::unique_ptr<T> candidate,
						Clock*, std::unique_ptr<T> (*) (char const*, std::unique_ptr<T>)){
			return candidate;
		}
		
		template<typename DS, typename T>
		std::unique_ptr<T>
		Tracker::onRead(TrackingContext &ctx, DS& ds, Name name,
						const Clock& version,
						std::unique_ptr<T> candidate,
						void*,
						std::unique_ptr<T> (*merge) (char const *,
													 std::unique_ptr<T>)){

			struct Owner {
				std::unique_ptr<T> candidate;
				std::unique_ptr<T> merged{nullptr};
				Owner(decltype(candidate) &candidate):candidate(std::move(candidate)){}
			};
			Owner mem{candidate};
			
			std::function<void (char const*)> c_merge{
				[merge,&mem](char const * arg){
				assert(mem.candidate);
				assert(!mem.merged);
				mem.merged = merge((char const *) arg, std::move(mem.candidate));
				}};
			
			onRead(ctx,ds,name,version,c_merge);
			if (mem.merged) return std::move(mem.merged);
			else return std::move(mem.candidate);
		}

	}}
