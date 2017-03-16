#pragma once
#include "Tracker.hpp"
#include <tuple>

namespace myria { namespace tracker { 



		//I'm just going to guess the names of the functions here.
		template<typename DS>
		auto wrapStore(DS &){
		  using l = typename DS::label;
		  using newTomb_t = typename Tracker::GenericTrackerDS::newTomb_t;
		  using existingClock_t = typename Tracker::GenericTrackerDS::existingClock_t;
		  using existingTomb_t = typename Tracker::GenericTrackerDS::existingTomb_t;
		  using exists_t = typename Tracker::GenericTrackerDS::exists_t;
		  static const newTomb_t newTomb = [](tracker::Tracker &trk, mtl::TransactionContext& ctx, GDataStore &_ds, Name name, auto &e){
		    auto &ds = dynamic_cast<DS&>(_ds);
		    return std::make_unique<LabelFreeHandle<Tracker::Tombstone> >
		    (new Handle<l,Tracker::Tombstone> (ds.template newObject<HandleAccess::all>(trk,&ctx, name,e)));
		  };
		  static const exists_t exists = [](GDataStore &_ds, Name name){
		    auto &ds = dynamic_cast<DS&>(_ds);
				return ds.exists(name);
		  };
		  static const existingClock_t existClock =
		    [](std::unique_ptr<mutils::abs_StructBuilder>& asb, GDataStore &_ds, Name name)
			  -> std::unique_ptr<TypedRemoteObject<Tracker::Clock> >{
		    auto &ds = dynamic_cast<DS&>(_ds);
		    return ds.template existingRaw<Tracker::Clock>(asb,name);
		  };
		  static const existingTomb_t existTomb =
		    [](std::unique_ptr<mutils::abs_StructBuilder>& asb, GDataStore &_ds, Name name)
		    -> std::unique_ptr<TypedRemoteObject<Tracker::Tombstone> >{
		    auto &ds = dynamic_cast<DS&>(_ds);
		    return ds.template existingRaw<Tracker::Tombstone>(asb,name);
		  };
		  return std::unique_ptr<GenericTrackerDS>
		    (new GenericTrackerDS{newTomb, exists,existClock,existTomb});
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
