#include "DataStore.hpp"
#include <cstdlib>
#include <time.h>
#include "GDataStore.hpp"
#include "compile-time-lambda.hpp"
#include "utils.hpp"
#include <functional>
#include <array>
#include <map>
#include <list>
#include <set>
#include <chrono>
#include <thread>
#include <unistd.h>

#include "CooperativeCache.hpp"
#include "Tracker_common.hpp"
#include "CompactSet.hpp"
#include "Ends.hpp"
#include "SafeSet.hpp"
#include "Ostreams.hpp"
#include "Tracker_private_declarations.hpp"
namespace {
constexpr unsigned long long bigprime_lin =
#include "big_prime"
    ;
}
#include <cstdlib>

namespace myria {
namespace tracker {

using namespace std;
using namespace chrono;
using namespace mutils;
using namespace mtl;
using namespace tracker;

Bundle::Bundle(std::future<CooperativeCache::obj_bundle> f)
    : f(new decltype(f)(std::move(f))) {}

Bundle::Bundle() {}

CooperativeCache::obj_bundle &Bundle::get() {
  if (f->valid()) {
    p = heap_copy(f->get());
  }
  assert(!f->valid());
  return *p;
}

Bundle::~Bundle() {
  // TODO: there's a memory error here somewhere.  Just leak for now.
  /*
    if (f->valid()){
    assert(f.use_count() > 0);
    destroyed_bundles()->emplace(std::async(std::launch::async,[f = this->f]()
    -> bool {
    while (f->wait_for(1ms) == future_status::timeout) {}
    return true;
    }));
    }//*/
}

void TrackingContext::commitContext() {
  i->_commitContext();
  i->_finalize();
}
void TrackingContext::abortContext() { i->_finalize(); }

TrackingContext::TrackingContext(Tracker &trk, GPhaseContext &ctx, bool cod)
    : i(new TrackingContext::Internals{*trk.i, cod}), trk(trk), ctx(ctx) {}

TrackingContext::~TrackingContext() {
  if (i)
    delete i;
}

std::unique_ptr<TrackingContext> Tracker::generateContext(GPhaseContext &ctx,
                                                          bool commitOnDelete) {
  return std::unique_ptr<TrackingContext>{
      (new TrackingContext{*this, ctx, commitOnDelete})};
}

namespace {
void remove_pending(TrackingContext::Internals &ctx, Tracker::Internals &i,
                    const Name &name) {
  ctx.pending_nonces_add.remove_if([&](auto &e) { return e.first == name; });
  i.pending_nonces.erase(name);
}

bool tracking_candidate(Tracker &t, Name name, const Tracker::Clock &version) {
  t.updateClock();
  if (!ends::is_same(version, {{-1, -1, -1, -1}}) &&
      ends::prec(version, t.i->global_min)) {
    t.i->tracking.erase(name);
    return false;
  } else
    return t.i->exceptions.count(name) == 0;
}
}

void Tracker::assert_nonempty_tracking() const {
  assert(!(i->tracking.empty()));
}

const CooperativeCache &Tracker::getcache() const { return i->cache; }

Tracker::Tracker(int cache_port, CacheBehaviors beh)
    : i{new Internals{beh}}, cache_port(cache_port) {
  assert(cache_port > 0 &&
         "error: must specify non-zero cache port for first tracker call");
  i->cache.listen_on(cache_port);
  // std::cout << "tracker built" << std::endl;
}

Tracker::~Tracker() { delete i; }

Name Tracker::Tombstone::name() const { return nonce; }

void Tracker::exemptItem(Name name) { i->exceptions.insert(name); }

namespace {

// constexpr int bigprime_causal = 2751103;

bool is_metaname(long int base, Name name) {
  return (name > 0) && ((name % base) == 0);
}

Name make_metaname(long int base, Name name) {
  assert([=]() {
    Name sanity = numeric_limits<int>::max() * base;
    return sanity > 0;
  }());
  assert(name <= numeric_limits<int>::max());
  assert(name > 0);
  assert(!is_metaname(base, name));
  Name cand = name * base;
  assert(cand > 0);
  assert(is_metaname(base, cand));
  return cand;
}

bool is_lin_metadata(Name name) { return is_metaname(bigprime_lin, name); }

Name make_lin_metaname(Name name) { return make_metaname(bigprime_lin, name); }

int get_ip() {
  static int ip_addr{[]() {
    std::string static_addr{MY_IP};
    if (static_addr.length() == 0)
      static_addr = "127.0.0.1";
    return (int)mutils::decode_ip(static_addr.c_str());
  }()};
  return ip_addr;
}
}
struct TombNameCollision {};

void Tracker::writeTombstone(tracker::Tracker &trk, mtl::GPhaseContext &ctx,
                             Tracker::Nonce nonce, Tracker::Internals &i) {
  const Tracker::Tombstone t{nonce, get_ip(), trk.cache_port};
  assert(i.cache.contains(nonce));
  assert(ctx.store_context());
  WeakTrackableDataStore &ds =
      dynamic_cast<WeakTrackableDataStore &>(ctx.store_context()->store());
  if (ds.exists(&ctx, t.name()))
    throw TombNameCollision{};
  else
    ds.new_tomb(&ctx, t.name(), t);
}

void Tracker::onStrongWrite(mtl::GPhaseContext &ctx, Name name) {
  const auto write_lin_metadata = [this](mtl::GPhaseContext &ctx,
                                         StrongTrackableDataStore &ds_real,
                                         Name name, Tracker::Nonce nonce) {
    assert(ctx.store_context());
    auto meta_name = make_lin_metaname(name);
    if (ds_real.exists(&ctx, meta_name)) {
      ds_real.existing_tombstone(&ctx, meta_name)
          ->put(&ctx, Tracker::Tombstone{nonce, get_ip(), cache_port});
    } else {
      ds_real.new_tomb(&ctx, meta_name,
                       Tracker::Tombstone{nonce, get_ip(), cache_port});
    }
    for (auto &p : i->tracking) {
      assert(p.second.second.data());
    }
    i->cache.insert(nonce, i->tracking);
    assert(i->cache.contains(nonce));
  };

  assert(ctx.store_context());
  auto &ds_real =
      dynamic_cast<StrongTrackableDataStore &>(ctx.store_context()->store());
  auto tracking_copy = i->tracking;
  for (auto &pair : tracking_copy) {
    // this will have the side effect of updating the clock,
    // and removing these items from the tracking set if the clock
    // is sufficiently recent.
    tracking_candidate(*this, pair.first, pair.second.first);
  }
  if (!is_lin_metadata(name) && !i->tracking.empty()) {

    auto subroutine = [&]() {
      auto nonce = long_rand();
      write_lin_metadata(ctx, ds_real, name, nonce);
    };
    bool always_failed = true;
    auto sleep_time = 2ms;
    for (int asdf = 0; asdf < 10; ++asdf) {
      try {
        subroutine();
        always_failed = false;
        break;
      } catch (const TombNameCollision &) {
        this_thread::sleep_for(sleep_time);
        sleep_time *= 2;
        // assume we picked a bad nonce, try again
      }
    }
    if (always_failed) {
      // it's almost certainly going to fail again, but at this point
      // we are really interested in what the error is.
      subroutine();
    }
    assert(!always_failed);
    assert(ds_real.exists(&ctx, make_lin_metaname(name)));
  }
}

std::ostream &operator<<(std::ostream &os, const Tracker::Clock &c) {
  os << "Clock: [";
  for (auto &a : c) {
    os << a << ",";
  }
  return os << "]";
}

bool sleep_on(TrackingContext::Internals &ctx, Tracker::Internals &i,
              GPhaseContext &pctx, WeakTrackableDataStore &ds,
              const Name &tomb_name, const int how_long = -1) {
  bool first_skip = true;
  for (int cntr = 0; (cntr < how_long) || how_long == -1; ++cntr) {
    if (ds.exists(&pctx, tomb_name)) {
      // if (!first_skip) std::cout << "done waiting" << std::endl;
      remove_pending(ctx, i, tomb_name);
      return true;
    } else {
      if (first_skip) {
        // std::cout << "waiting for " << tomb_name << " to appear..." <<
        // std::endl;
        first_skip = false;
      }
      std::this_thread::sleep_for(10ms);
    }
  }
  return false;
}

template <typename P>
std::vector<char> const *
wait_for_available(TrackingContext::Internals &ctx, Tracker::Internals &i,
                   GPhaseContext &pctx, WeakTrackableDataStore &ds, Name name,
                   P &p, const Tracker::Clock &v) {
  if (ds.exists(&pctx, p.first)) {
    remove_pending(ctx, i, p.first);
    return nullptr;
  } else {
    try {
      auto &remote = p.second.get();
      auto ret = CooperativeCache::find(remote, name, v);
      assert(ret->data());
      return ret;
    } catch (const std::exception &e) {
      // something went wrong with the cooperative caching
      // std::cout << "Cache request failed! Waiting for tombstone" <<
      // std::endl;
      // std::cout << "error message: " << e.what() << std::endl;

      sleep_on(ctx, i, pctx, ds, p.first);
      assert(ds.exists(&pctx, p.first));
      remove_pending(ctx, i, p.first);
      return nullptr;
    } catch (...) {
      std::cerr << "this is a very bad error" << std::endl;
      assert(false && "whaaat");
    }
  }
}

void Tracker::afterStrongRead(mtl::GPhaseContext &sctx, Name name) {

  TrackingContext &tctx = sctx.trk_ctx;
  assert(name != 1);
  assert(sctx.store_context());
  StrongTrackableDataStore &ds =
      dynamic_cast<StrongTrackableDataStore &>(sctx.store_context()->store());

  if (!is_lin_metadata(name)) {
    updateClock();
    auto ts = make_lin_metaname(name);
    if (ds.exists(&sctx, ts)) {
      auto tomb_p = ds.existing_tombstone(&sctx, ts)->get(&sctx);
      auto &tomb = *tomb_p;
      // std::cout << "Nonce isn't immediately available, adding to
      // pending_nonces" << std::endl;
      tctx.i->pending_nonces_add.emplace_back(tomb.name(),
                                              Bundle{i->cache.get(tomb)});
    }
  }
}

#define for_each_pending_nonce(ctx, i, f...)                                   \
  {                                                                            \
    for (auto &p : i->pending_nonces) {                                        \
      f /*(p)*/;                                                               \
    }                                                                          \
    for (auto &p : ctx->pending_nonces_add) {                                  \
      f /*(p)*/;                                                               \
    }                                                                          \
  }

// for when merging locally is too hard or expensive.  Returns "true" when
// candidate version is fine to return, "false" otherwise
bool Tracker::waitForCausalRead(mtl::GPhaseContext &ctx, Name name,
                                const Clock &version) {
  // TODO: distinctly not thread-safe
  // if the user called onRead manually and did a merge,
  // we don't want to wait here.This has been ongoing for a couple weeks
  // since this is always called directly after
  // the user had the chance to use onRead,
  // we can use this trivial state tracking mechanism
  // std::cout << "break 1!" << std::endl;
  if (i->last_onRead_name && *i->last_onRead_name == name) {
    // std::cout << "break 1: we used onRead, so we're skipping this" <<
    // std::endl;
    i->last_onRead_name.reset(nullptr);
    return true;
  }

  auto &tracking_context = ctx.trk_ctx;
  assert(ctx.store_context());
  auto &ds =
      dynamic_cast<WeakTrackableDataStore &>(ctx.store_context()->store());

  if (tracking_candidate(*this, name, version)) {
    // std::cout << "break 1: this is a tracking candidate" << std::endl;
    // need to pause here and wait for nonce availability
    // for each nonce in the list

    {
      for (auto &p : i->pending_nonces) {
        // std::cout << "break 1: checking wait_for_available" << std::endl;
        if (wait_for_available(*tracking_context.i, *i, ctx, ds, name, p,
                               version)) {
          // std::cout << "break 1: wait_for_available in if-condition" <<
          // std::endl;
          // I know we've gotten a cached version of the object,
          // but we can't merge it, so we're gonna have to
          // hang out until we've caught up to the relevant tombstone
          // std::cout << "Cache request succeeded!  But we don't know how to
          // merge.."
          //		  << std::endl;
          sleep_on(*tracking_context.i, *i, ctx, ds, p.first);
          assert(ds.exists(&ctx, p.first));
        }
      }
      for (auto &p : tracking_context.i->pending_nonces_add) {
        // std::cout << "break 1: checking wait_for_available" << std::endl;
        if (wait_for_available(*tracking_context.i, *i, ctx, ds, name, p,
                               version)) {
          // std::cout << "break 1: wait_for_available in if-condition" <<
          // std::endl;
          // I know we've gotten a cached version of the object,
          // but we can't merge it, so we're gonna have to
          // hang out until we've caught up to the relevant tombstone
          // std::cout << "Cache request succeeded!  But we don't know how to
          // merge.."
          //		  << std::endl;
          sleep_on(*tracking_context.i, *i, ctx, ds, p.first);
          assert(ds.exists(&ctx, p.first));
        }
      }
    }
    return false;
  }
  return true;
}

void Tracker::afterCausalRead(TrackingContext &tctx, Name name,
                              const Clock &version,
                              const std::vector<char> &data) {
  if (tracking_candidate(*this, name, version)) {
    // need to overwrite, not occlude, the previous element.
    // C++'s map semantics are really stupid.
    tctx.i->tracking_erase.push_back(name);
    assert(data.data());
    assert(data.size() > 0);
    // std::cout << data << std::endl;
    for (auto &i : version)
      assert(i != -1);
    if (!ends::prec(version, i->global_min)) {
      tctx.i->tracking_add.emplace_back(name, std::make_pair(version, data));
      // std::cout << tctx.i->tracking_add.back().second.second << std::endl;
      assert(tctx.i->tracking_add.back().second.second.data());
    }
  }
}

// for when merging is the order of the day
void Tracker::onCausalRead(
    GPhaseContext &pctx, Name name, const Clock &version,
    const std::function<void(char const *)> &construct_and_merge) {
  TrackingContext &ctx = pctx.trk_ctx;
  assert(pctx.store_context());
  auto &ds =
      dynamic_cast<WeakTrackableDataStore &>(pctx.store_context()->store());
  i->last_onRead_name = heap_copy(name);
  if (tracking_candidate(*this, name, version)) {
    // need to pause here and wait for nonce availability
    // for each nonce in the list
    for_each_pending_nonce(ctx.i, i,
                           if (auto *remote_vers = wait_for_available(
                                   *ctx.i, *i, pctx, ds, name, p, version)) {
                             // build + merge real object
                             assert(remote_vers->data());
                             construct_and_merge(remote_vers->data());
                             return;

                             /**
                                There are many pending nonces; any of them could
                                be in our tracking
                                set
                                due to a dependency on this object. Cycle
                                through them until we
                                find one
                                that is (or fail to find any that are); this one
                                will tell us a
                                place to get
                                the object from the cooperative cache. Grab the
                                object from the
                                cache there.
                                If that fails, then too bad; handling that comes
                                later.
                              */
                           } else assert(ds.exists(&pctx, p.first)););
  }
  return;
}
}
}
