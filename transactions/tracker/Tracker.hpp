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
#include "TransactionBasics.hpp"
#include "TrackingContext.hpp"

namespace myria { 

	template<Level l, HandleAccess HA, typename T>
	struct Handle;
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
			struct Tombstone {
				const Nonce nonce;
				const int ip_addr;
				const int portno;
				Name name() const;

				/*
				int to_bytes(char* v) const;
				int bytes_size() const;
				static std::unique_ptr<Tombstone> from_bytes(DeserializationManager*,char const* v);
				Tombstone(Nonce n,int ip, int portno):nonce(n),ip_addr(ip),portno(portno){}
				Tombstone(const Tombstone& t):nonce(t.nonce),ip_addr(t.ip_addr),portno(t.portno){} //*/
			};

			using Clock = std::array<int,NUM_CAUSAL_GROUPS>;
			template<Level l>
			using TrackerDS =
				std::tuple<
				Handle<l, HandleAccess::all, Tombstone> (*)
				(tracker::Tracker &trk, mtl::TransactionContext& ctx, DataStore<l>&, Name, const Tombstone&), //newTomb
				bool (*) (DataStore<l>&, Name), //exists
				std::unique_ptr<RemoteObject<l, Clock> > (*)
				(DataStore<l>&, Name), //existingClock
				std::unique_ptr<RemoteObject<l, Tombstone> > (*)
				(DataStore<l>&, Name) //existingTomb
				>;
			using TrackerDSStrong = TrackerDS<Level::strong>;
			using TrackerDSCausal = TrackerDS<Level::causal>;
			using StampedObject = mutils::TrivialTriple<Name, Tracker::Clock, std::vector<char> >;

			//hiding private members of this class. No implementation available.
			struct Internals;
			
		private:
			Internals *i;
			void onRead(
				TrackingContext&,
				DataStore<Level::causal>&, Name name, const Clock &version,
				const std::function<void (char const *)> &construct_nd_merge);
			void onRead(
				TrackingContext&,
				DataStore<Level::strong>&, Name name, const Clock &version,
				const std::function<void (char const *)> &construct_nd_merge);
	
		public:
			//static Tracker& global_tracker(int cache_port = -1);

			bool registered(const GDataStore&) const;

			const DataStore<Level::strong>& get_StrongStore() const;
			const DataStore<Level::causal>& get_CausalStore() const;

			void registerStore(DataStore<Level::strong> &,
							   std::unique_ptr<TrackerDSStrong>);
			void registerStore(DataStore<Level::causal> &,
							   std::unique_ptr<TrackerDSCausal>);

			template<typename DS>
			void registerStore(DS &ds);

			void exemptItem(Name name);

			template<typename T, Level l, HandleAccess ha>
			void exemptItem(const Handle<l,ha,T>& h){
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

			void onWrite(mtl::TransactionContext&, DataStore<Level::strong>&, Name name, Tombstone*);
			void onWrite(mtl::TransactionContext&, DataStore<Level::strong>&, Name name, Clock*);
			void onWrite(mtl::TransactionContext&, DataStore<Level::strong>&, Name name, void*);
			

			void onWrite(DataStore<Level::causal>&, Name name, const Clock &version, Tombstone*);
			void onWrite(DataStore<Level::causal>&, Name name, const Clock &version, Clock*);
			void onWrite(DataStore<Level::causal>&, Name name, const Clock &version, void*);

			void onCreate(DataStore<Level::causal>&, Name name,Tombstone*);
			void onCreate(DataStore<Level::causal>&, Name name,Clock*);
			void onCreate(DataStore<Level::causal>&, Name name,void*);
			
			void onCreate(DataStore<Level::strong>&, Name name, Tombstone*);
			void onCreate(DataStore<Level::strong>&, Name name, Clock*);
			void onCreate(DataStore<Level::strong>&, Name name, void*);

			void afterRead(mtl::StoreContext<Level::strong>&, TrackingContext&, 
						   DataStore<Level::strong>&, Name name, Tombstone*);
			void afterRead(mtl::StoreContext<Level::strong>&, TrackingContext&, 
						   DataStore<Level::strong>&, Name name, Clock*);
			void afterRead(mtl::StoreContext<Level::strong>&, TrackingContext&, 
						   DataStore<Level::strong>&, Name name, void*);

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
			bool waitForRead(TrackingContext&, DataStore<Level::causal>&, Name name, const Clock& version, Tombstone*);
			bool waitForRead(TrackingContext&, DataStore<Level::causal>&, Name name, const Clock& version, Clock*);
			bool waitForRead(TrackingContext&, DataStore<Level::causal>&, Name name, const Clock& version, void*);

			void afterRead(TrackingContext&, DataStore<Level::causal>&, Name name, const Clock& version, const std::vector<char> &data, Tombstone*);
			void afterRead(TrackingContext&, DataStore<Level::causal>&, Name name, const Clock& version, const std::vector<char> &data, Clock*);
			void afterRead(TrackingContext&, DataStore<Level::causal>&, Name name, const Clock& version, const std::vector<char> &data, void*);

			//for testing
			void assert_nonempty_tracking() const;
			const CooperativeCache& getcache() const;

			friend struct TrackingContext;

			Tracker(int cache_port, std::function<std::ostream& ()> logger, CacheBehaviors behavior /*= CacheBehaviors::full*/);
			virtual ~Tracker();

			Tracker(const Tracker&) = delete;

			const int cache_port;
			const std::function<std::ostream& ()> logger;

		};		

	}}
