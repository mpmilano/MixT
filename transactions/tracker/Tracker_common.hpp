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
			static newTomb_t newTomb = [](tracker::Tracker &trk, mtl::TransactionContext& ctx, DataStore<l> &_ds, Name name, auto &e){
				auto &ds = dynamic_cast<DS&>(_ds);
				return ds.template newObject<HandleAccess::all>(trk,&ctx, name,e);
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

		template<typename DS, typename T>
		std::unique_ptr<T>
		Tracker::onRead(TrackingContext &ctx, DS& ds, Name name,
						const Clock& version,
						std::unique_ptr<T> candidate,
						std::unique_ptr<T> (*merge) (std::unique_ptr<T>,
													 std::unique_ptr<T>)){

			struct Owner : public MemoryOwner {
				std::unique_ptr<T> candidate;
				std::unique_ptr<T> merged;
				Owner(decltype(candidate) &candidate):candidate(std::move(candidate)){}
			};
			std::shared_ptr<std::unique_ptr<T> >
				cand{new std::unique_ptr<T>(std::move(candidate))};
			std::function<std::unique_ptr<MemoryOwner> ()> mem = 
				[=]() {
				assert(*cand);
				return std::unique_ptr<MemoryOwner>(new Owner(*cand));
			};
			std::function<void (MemoryOwner&, char const*)> c_merge{
				[merge](MemoryOwner& mo, char const * arg){
				auto& o = dynamic_cast<Owner&>(mo);
				assert(o.candidate);
				o.merged = merge(mutils::from_bytes<T>((char const *) arg),
								 std::move(o.candidate));
				}};
			auto mo = onRead(ctx,ds,name,version,mem,c_merge);
			if (auto *o = dynamic_cast<Owner*>(mo.get()))
				return std::move(o->merged);
			else {
				auto mo = mem();
				return std::move(dynamic_cast<Owner&>(*mo).candidate);
			}
		}

	}}
