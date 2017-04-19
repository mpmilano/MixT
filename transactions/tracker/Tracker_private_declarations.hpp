#pragma once

#include "Tracker.hpp"
#include <thread>
#include <future>
#include "DataStore.hpp"

namespace myria {
namespace tracker {

struct Tracker::Internals {
  Internals(const Internals &) = delete;
	std::unique_ptr<std::vector<Tombstone> > encountered_tombstones{new std::vector<Tombstone>{}};
  Clock global_min{{0, 0, 0, 0}};
  Internals() = default;
};

struct TrackingContext::Internals {
  Tracker &trk_sup;
  Tracker::Internals &trk;

  Internals(Tracker &trk_sup, Tracker::Internals &trk)
      : trk_sup(trk_sup), trk(trk) {
    trk_sup.updateClock();
  }

  // std::list<std::pair<Name,Bundle> >pending_nonces_add;
	std::unique_ptr<std::vector<Tombstone> > pending_nonces{new std::vector<Tombstone>{}};
	void _commitContext(){
		assert(!trk.encountered_tombstones || trk.encountered_tombstones->size() == 0);
		trk.encountered_tombstones = std::move(pending_nonces);
	}

  virtual ~Internals() = default;
};
}
}
#include "phase_context.hpp"
