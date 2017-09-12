#pragma once

#include "tracker/Tracker.hpp"
#include <thread>
#include <future>
#include "DataStore.hpp"

namespace myria {
namespace tracker {

struct Tracker::Internals {
  Internals(const Internals &) = delete;
	std::unique_ptr<std::vector<Tombstone> > encountered_tombstones{new std::vector<Tombstone>{}};
  Clock global_min{{0, 0, 0, 0}};
	Clock new_objects_max{{0,0,0,0}};
	TrackableDataStore_super *persistent_datastore{nullptr};
  Internals() = default;
};

struct TrackingContext::Internals {
  Tracker &trk_sup;
  Tracker::Internals &trk;

  Internals(Tracker &trk_sup, Tracker::Internals &trk)
      : trk_sup(trk_sup), trk(trk) {
    trk_sup.updateClock();
  }

	std::unique_ptr<std::vector<Tombstone> > pending_nonces{new std::vector<Tombstone>{}};
	std::vector<Clock> newer_objects;
	void _commitContext(){
		using namespace ends;
		assert(!trk.encountered_tombstones || trk.encountered_tombstones->size() == 0);
		trk.encountered_tombstones = std::move(pending_nonces);
		trk.new_objects_max = ends::max(
			trk.new_objects_max,
			ends::max(newer_objects.begin(),newer_objects.end()));
	}

  virtual ~Internals() = default;
};
}
}
#include "mtl/phase_context.hpp"
