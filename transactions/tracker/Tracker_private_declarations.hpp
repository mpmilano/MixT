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
  Bundle(std::future<CooperativeCache::obj_bundle> f);

  Bundle();
  virtual ~Bundle();

  CooperativeCache::obj_bundle &get();
};

struct Tracker::Internals {
  Internals(const Internals &) = delete;

  Clock global_min{{0, 0, 0, 0}};

  std::map<Name, std::pair<Clock, std::vector<char>>> tracking;
  std::map<Name, Bundle> pending_nonces;
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
      tracker.pending_nonces.emplace(tomb.name(), Bundle{trk.cache.get(tomb)});
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

template <typename DS>
std::unique_ptr<LabelFreeHandle<tracker::Tombstone>>
TrackableDataStore_common<DS>::new_tomb(mtl::GPhaseContext *_ctx, Name n,
                                        const tracker::Tombstone &val) {
  using namespace tracker;
  DS *ds = dynamic_cast<DS *>(this);
  assert(ds);
  typename DS::StoreContext *ctx =
      ((mtl::PhaseContext<label> *)_ctx)
          ->store_context(*ds whendebug(, "tracker wants a new tombstone"));
  auto ret = ds->newObject(ctx, n, val);
  // erase operation support, if any
  Handle<label, Tombstone> h = ret;
  return std::unique_ptr<LabelFreeHandle<tracker::Tombstone>>{new DECT(h){h}};
}
template <typename DS>
bool TrackableDataStore_common<DS>::exists(mtl::GPhaseContext *_ctx, Name n) {
  using namespace tracker;
  DS *ds = dynamic_cast<DS *>(this);
  assert(ds);
  typename DS::StoreContext *ctx =
      ((mtl::PhaseContext<label> *)_ctx)
          ->store_context(
              *ds whendebug(, "tracker wants to see if something exists"));
  return ds->exists(ctx, n);
}

template <typename DS>
std::unique_ptr<LabelFreeHandle<tracker::Clock>>
TrackableDataStore_common<DS>::existing_clock(mtl::GPhaseContext *_ctx,
                                              Name n) {
  using namespace tracker;
  DS *ds = dynamic_cast<DS *>(this);
  assert(ds);
  typename DS::StoreContext *ctx =
      ((mtl::PhaseContext<label> *)_ctx)
          ->store_context(*ds whendebug(, "tracker wants an existing clock"));
  auto ret = ds->template existingObject<Clock>(ctx, n);
  // erase operation support, if any
  Handle<label, Clock> h = ret;
  return std::unique_ptr<LabelFreeHandle<Clock>>{new DECT(h){h}};
}
template <typename DS>
std::unique_ptr<LabelFreeHandle<tracker::Tombstone>>
TrackableDataStore_common<DS>::existing_tombstone(mtl::GPhaseContext *_ctx,
                                                  Name n) {
  using namespace tracker;
  DS *ds = dynamic_cast<DS *>(this);
  assert(ds);
  typename DS::StoreContext *ctx =
      ((mtl::PhaseContext<label> *)_ctx)
          ->store_context(
              *ds whendebug(, "tracker wants an existing tombstone"));
  auto ret = ds->template existingObject<Tombstone>(ctx, n);
  // erase operation support, if any
  Handle<label, Tombstone> h = ret;
  return std::unique_ptr<LabelFreeHandle<Tombstone>>{new DECT(h){h}};
}
}
#include "phase_context.hpp"
