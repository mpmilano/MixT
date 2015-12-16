//What goes here: managed tracking of stores, explicitly for
//cross-store stuff.

#pragma once

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

namespace myria { 

	template<Level l, HandleAccess HA, typename T>
	struct Handle;
	namespace tracker {

		namespace TDS{
			static constexpr int newTomb = 0;
			static constexpr int exists = 1;
			static constexpr int existingClock = 2;
			static constexpr int existingTomb = 3;
		}

		struct TrackingContext;

		class Tracker {
		public:
			//support structures, metadata.
			using Nonce = int;
			struct Tombstone{
				Nonce nonce;
				int ip_addr;
				Name name() const;
			};

			using Clock = std::array<int,NUM_CAUSAL_GROUPS>;
			template<Level l>
			using TrackerDS =
				std::tuple<
				Handle<l, HandleAccess::all, Tombstone> (*)
				(DataStore<l>&, Name, const Tombstone&), //newTomb
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
			struct MemoryOwner {
				MemoryOwner(const MemoryOwner&) = delete;
				MemoryOwner(){}
				virtual ~MemoryOwner(){}
			};
			std::unique_ptr<MemoryOwner> onRead(
				DataStore<Level::causal>&, Name name, const Clock &version,
				const std::function<std::unique_ptr<MemoryOwner> ()> &mem,
				const std::function<void (MemoryOwner&, char const *)> &construct_nd_merge);
			std::unique_ptr<MemoryOwner> onRead(
				DataStore<Level::strong>&, Name name, const Clock &version,
				const std::function<std::unique_ptr<MemoryOwner> ()> &mem,
				const std::function<void (MemoryOwner&, char const *)> &construct_nd_merge);
	
		public:
			static Tracker& global_tracker(int cache_port = -1);

			bool registered(const GDataStore&) const;

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

			TrackingContext& generateContext();
	
			void onWrite(DataStore<Level::strong>&, Name name);

			void onWrite(DataStore<Level::causal>&, Name name, const Clock &version);

			void onCreate(DataStore<Level::causal>&, Name name);

			void onCreate(DataStore<Level::strong>&, Name name);

			void afterRead(mtl::TransactionContext &tc,
						   DataStore<Level::strong>&, Name name);

			//return is non-null when read value cannot be used.
			template<typename DS, typename T>
			std::unique_ptr<T>
			onRead(DS&, Name name,
				   const Clock& version,
				   std::unique_ptr<T> candidate,
				   std::unique_ptr<T> (*merge)(std::unique_ptr<T>,
											   std::unique_ptr<T>)
				   = [](std::unique_ptr<T>,std::unique_ptr<T> r){return r;}
				);

			//for when merging locally is too hard or expensive
			bool waitForRead(DataStore<Level::causal>&, Name name, const Clock& version);

			void afterRead(DataStore<Level::causal>&, Name name, const Clock& version, const std::vector<char> &data);

			//for testing
			void assert_nonempty_tracking() const;

		private:
			const int cache_port;
			Tracker(int cache_port);
			virtual ~Tracker();

			Tracker(const Tracker&) = delete;
		};


	}}
