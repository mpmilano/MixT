#pragma once

#include "Tracker.hpp"
#include <thread>
#include <future>
#include "CooperativeCache.hpp"

namespace myria { namespace tracker {

		struct Bundle{
		private:
			std::shared_ptr<std::future<CooperativeCache::obj_bundle> > f;
			std::shared_ptr<CooperativeCache::obj_bundle> p;
			/*
			  static std::shared_ptr<MonotoneSafeSet<std::future<bool> > >& destroyed_bundles(bool reset = false){
			  static auto log = make_shared<MonotoneSafeSet<std::future<bool> > >();
			  static bool run_once = [](){std::thread(Bundle::cleanup_loop).detach();return true;}();
			  assert(run_once);
			  if (reset) log = make_shared<MonotoneSafeSet<std::future<bool> > >();
			  return log;
			  }
				static void cleanup_loop(){
				auto waiting_collection = destroyed_bundles();
				destroyed_bundles(true);
				std::this_thread::sleep_for(5ms); //well this is bad practice
				for (auto &f : waiting_collection->iterable_reference()){
				if (f.wait_for(1ms) != future_status::timeout) assert(f.get());
				else destroyed_bundles()->emplace(std::move(f));
				}
				}*/
			
		public:
			Bundle(std::future<CooperativeCache::obj_bundle> f);
			
			Bundle();
			virtual ~Bundle();
			
			CooperativeCache::obj_bundle& get();
		};
		
		
		struct Tracker::Internals{
			Internals(const Internals&) = delete;
			GDataStore *registeredStrong {nullptr};
			std::unique_ptr<TrackerDSStrong > strongDS;

			GDataStore *registeredCausal {nullptr};
			std::unique_ptr<TrackerDSCausal > causalDS;

			Clock global_min{{0,0,0,0}};

			std::map<Name, std::pair<Clock, std::vector<char> > > tracking;
			std::map<Name, Bundle> pending_nonces;
			std::set<Name> exceptions;
			CooperativeCache cache;
			std::unique_ptr<Name> last_onRead_name{nullptr};

			Internals(CacheBehaviors beh):cache(beh){}
		};

		struct TrackingContext::Internals {
			Tracker::Internals &trk;
			bool commitOnDelete;

			Internals(Tracker::Internals& trk, bool cod)
				:trk(trk),commitOnDelete(cod){}
			
			std::list<Name> tracking_erase;
			std::list<std::pair<Name,std::pair<Tracker::Clock,std::vector<char>> > > tracking_add;
			std::list<std::pair<Name,Bundle> >pending_nonces_add;

			auto _commitContext(){
				auto &tracker = trk;
				for (auto &e : tracking_erase){
					tracker.tracking.erase(e);
				}
				for (auto &e : tracking_add){
					tracker.tracking.emplace(e);
				}
				for (auto &e : pending_nonces_add){
					tracker.pending_nonces.emplace(e);
				}
			}

			auto _finalize(){
				commitOnDelete = false;
			}
			virtual ~Internals(){
				if (commitOnDelete){
					_commitContext();
				}
			}
		};

	}}
