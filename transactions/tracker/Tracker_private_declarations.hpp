#pragma once

#include "Tracker.hpp"
#include <thread>
#include <future>
#include "CooperativeCache.hpp"
#include "DataStore.hpp"

namespace myria {
namespace tracker {

struct Bundle {
private:
  std::shared_ptr<std::future<CooperativeCache::obj_bundle>> f;
  std::shared_ptr<CooperativeCache::obj_bundle> p;

public:
  const int ip_addr;
  const int portno;
  Bundle(int ip_addr, int portno, std::future<CooperativeCache::obj_bundle> f);

  virtual ~Bundle();

  CooperativeCache::obj_bundle &get();
};

struct Tracker::Internals {
  Internals(const Internals &) = delete;

  Clock global_min{{0, 0, 0, 0}};

  std::map<Name, std::pair<Clock, std::vector<char>>> tracking;
  std::map<Name, Bundle> pending_nonces;
  std::set<Name> already_seen_nonces;
  std::set<Name> exceptions;
  CooperativeCache cache;
  std::unique_ptr<Name> last_onRead_name{nullptr};

  Internals(CacheBehaviors beh) : cache(beh) {}
};

struct TrackingContext::Internals {
  Tracker &trk_sup;
  Tracker::Internals &trk;
  bool commitOnDelete;

  Internals(Tracker &trk_sup, Tracker::Internals &trk, bool cod)
      : trk_sup(trk_sup), trk(trk), commitOnDelete(cod) {
    trk_sup.updateClock();
  }

  std::list<Name> tracking_erase;
  std::list<std::pair<Name, std::pair<Tracker::Clock, std::vector<char>>>>
      tracking_add;
  // std::list<std::pair<Name,Bundle> >pending_nonces_add;
  std::list<Tombstone> pending_nonces_add;

  auto _commitContext() {
    trk_sup.updateClock();
    auto &tracker = trk;
    for (auto &e : tracking_erase) {
      tracker.tracking.erase(e);
    }
    for (auto &e : tracking_add) {
      tracker.tracking.emplace(e);
    }
    for (auto &tomb : pending_nonces_add) {
      tracker.pending_nonces.emplace(tomb.name(), Bundle{tomb.ip_addr,tomb.portno,trk.cache.get(tomb)});
    }
  }

  auto _finalize() { commitOnDelete = false; }
  virtual ~Internals() {
    if (commitOnDelete) {
      _commitContext();
    }
  }
};
}
}
#include "phase_context.hpp"
