//What goes here: managed tracking of stores, explicitly for
//cross-store stuff.

#pragma once

//#define MAKE_CACHE_REQUESTS yay
//#define ACCEPT_CACHE_REQUESTS yay

#include "CompactSet.hpp"
#include "GDataStore.hpp"
#include "compile-time-lambda.hpp"
#include "utils.hpp"
#include <functional>
#include <time.h>
#include "TrivialPair.hpp"
#include "RemoteObject.hpp"
#include "Ends.hpp"
#include "TransactionContext.hpp"
#include "TrackingContext.hpp"
#include "ObjectBuilder.hpp"
#include "Tombstone.hpp"

namespace myria { 

	template<typename l, typename T,typename... Ops>
	struct Handle;

	template<typename T>
  struct LabelFreeHandle;
	namespace tracker {

		class CooperativeCache;

		enum CacheBehaviors{
			full,onlymake,onlyaccept,none
		};

		namespace TDS{
			static constexpr int newTomb = 0;
			static constexpr int exists = 1;
			static constexpr int existingClock = 2;
			static constexpr int existingTomb = 3;
		}

		class Tracker {
		public:
			//support structures, metadata.
			using Nonce = int;
		  using Tombstone = tracker::Tombstone;

		  using Clock = std::array<int,NUM_CAUSAL_GROUPS>;
		  
		  struct GenericTrackerDS {
		    using newTomb_t = std::unique_ptr<LabelFreeHandle<Tombstone> > (*) (void* ctx, GDataStore&, Name, const Tombstone&);
		    newTomb_t newTomb;
		    using exists_t = bool (*) (GDataStore&, Name);
		    exists_t exists;
		    using existingClock_t = std::unique_ptr<TypedRemoteObject<Clock> > (*) (GDataStore&, Name);
		    existingClock_t existingClock;
		    using existingTomb_t = std::unique_ptr<TypedRemoteObject<Tombstone> > (*) (GDataStore&, Name);
		    existingTomb_t existingTomb;
		    virtual ~GenericTrackerDS() = default;
		    GenericTrackerDS(newTomb_t newTomb, exists_t exists, existingClock_t existingClock, existingTomb_t existingTomb)
		      :newTomb(newTomb),exists(exists),existingClock(existingClock),existingTomb(existingTomb){}
		  };

		  
		  using StampedObject = mutils::TrivialTriple<Name, Tracker::Clock, std::vector<char> >;

		  //hiding private members of this class. No implementation available.
		  struct Internals;
		  Internals *i;
		  
		private:
		  void onRead(
			      TrackingContext&,
			      GDataStore&, Name name, const Clock &version,
			      const std::function<void (char const *)> &construct_nd_merge, std::true_type* requires_tracking);
		  void onRead(
			      TrackingContext&,
			      GDataStore&, Name name, const Clock &version,
			      const std::function<void (char const *)> &construct_nd_merge, std::false_type* requires_tracking);
		  
		public:
			static constexpr int clockport = 9999;
                        void updateClock();
	
			//static Tracker& global_tracker(int cache_port = -1);

			bool registered(const GDataStore&) const;

			/*
			const GDataStore& get_StrongStore() const;
			const GDataStore& get_CausalStore() const;
			GDataStore& get_StrongStore();
			GDataStore& get_CausalStore();

			bool strongRegistered() const;
			bool causalRegistered() const;


			template<typename DS>
			void registerStore(DS &ds);

			void registerStore(GDataStore &,
												 std::unique_ptr<GenericTrackerDS>, std::true_type*);
			void registerStore(GDataStore &,
												 std::unique_ptr<GenericTrackerDS>, std::false_type*);
//*/

			void exemptItem(Name name);

			template<typename T, typename l, typename... Ops>
			void exemptItem(const Handle<l,T,Ops...>& h){
				exemptItem(h.name());
			}

			std::unique_ptr<TrackingContext> generateContext(bool commitOnDelete = false);

			/**
			   The primary interface methods here are tripled.  
			   The purpose of doing this is to special-case the Tracker's own
			   datastructures; we don't really want to invoke tracking code on the
			   nonces or tombstones, since those are either guaranteed unique or 
			   guaranteed up-to-date.
			 */

			void onStrongWrite(GDataStore&, Name name, Tombstone*);
			void onStrongWrite(GDataStore&, Name name, Clock*);
			void onStrongWrite(GDataStore&, Name name, void*);
			

			void onCausalWrite(GDataStore&, Name name, const Clock &version, Tombstone*);
			void onCausalWrite(GDataStore&, Name name, const Clock &version, Clock*);
			void onCausalWrite(GDataStore&, Name name, const Clock &version, void*);

			void onCausalCreate(GDataStore&, Name name,Tombstone*);
			void onCausalCreate(GDataStore&, Name name,Clock*);
			void onCausalCreate(GDataStore&, Name name,void*);
			
			void onStrongCreate(GDataStore&, Name name, Tombstone*);
			void onStrongCreate(GDataStore&, Name name, Clock*);
			void onStrongCreate(GDataStore&, Name name, void*);

			void afterStrongRead(mtl::GStoreContext&, TrackingContext&, 
						   GDataStore&, Name name, Tombstone*);
			void afterStrongRead(mtl::GStoreContext&, TrackingContext&, 
						   GDataStore&, Name name, Clock*);
			void afterStrongRead(mtl::GStoreContext&, TrackingContext&, 
						   GDataStore&, Name name, void*);

			//return is non-null when read value cannot be used.
			template<typename DS, typename T>
			std::unique_ptr<T>
			onRead(TrackingContext&, DS&, Name name,
				   const Clock& version,
				   std::unique_ptr<T> candidate,
				   Tombstone*,
				   std::unique_ptr<T> (*merge)(char const *,
							       std::unique_ptr<T>)
			       = [](char const *,std::unique_ptr<T> r){return r;}
			       );

			//return is non-null when read value cannot be used.
			template<typename DS, typename T>
			std::unique_ptr<T>
			onRead(TrackingContext&, DS&, Name name,
				   const Clock& version,
				   std::unique_ptr<T> candidate,
				   Clock*,
				   std::unique_ptr<T> (*merge)(char const *,
							       std::unique_ptr<T>)
			       = [](char const *,std::unique_ptr<T> r){return r;}
			       );

		  //return is non-null when read value cannot be used.
			template<typename DS, typename T>
			std::unique_ptr<T>
			onRead(TrackingContext&, DS&, Name name,
				   const Clock& version,
				   std::unique_ptr<T> candidate,
				   void*,
				   std::unique_ptr<T> (*merge)(char const*,
							       std::unique_ptr<T>)
				   = [](char const*,std::unique_ptr<T> r){return r;}
				);

			//for when merging locally is too hard or expensive
			bool waitForCausalRead(TrackingContext&, GDataStore&, Name name, const Clock& version, Tombstone*);
			bool waitForCausalRead(TrackingContext&, GDataStore&, Name name, const Clock& version, Clock*);
			bool waitForCausalRead(TrackingContext&, GDataStore&, Name name, const Clock& version, void*);

			void afterCausalRead(TrackingContext&, GDataStore&, Name name, const Clock& version, const std::vector<char> &data, Tombstone*);
			void afterCausalRead(TrackingContext&, GDataStore&, Name name, const Clock& version, const std::vector<char> &data, Clock*);
			void afterCausalRead(TrackingContext&, GDataStore&, Name name, const Clock& version, const std::vector<char> &data, void*);

			//for testing
			void assert_nonempty_tracking() const;
			const CooperativeCache& getcache() const;

			friend struct TrackingContext;

			Tracker(int cache_port, CacheBehaviors behavior /*= CacheBehaviors::full*/);
			virtual ~Tracker();

			Tracker(const Tracker&) = delete;

			const int cache_port;

		};		

	}}
