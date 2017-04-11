// What goes here: managed tracking of stores, explicitly for
// cross-store stuff.

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

template <typename l, typename T, typename... Ops> struct Handle;

template <typename T> struct LabelFreeHandle;
namespace tracker {

class CooperativeCache;

enum CacheBehaviors { full, onlymake, onlyaccept, none };

class Tracker {
public:
  // support structures, metadata.
  using Tombstone = tracker::Tombstone;
  using Clock = tracker::Clock;
  using Nonce = int;

  using StampedObject =
      mutils::TrivialTriple<Name, Tracker::Clock, std::vector<char>>;

  // hiding private members of this class. No implementation available.
  struct Internals;
  Internals *i;

  static constexpr int clockport = 9999;
  void updateClock();

  void exemptItem(Name name);

  template <typename T, typename l, typename... Ops>
  void exemptItem(const Handle<l, T, Ops...> &h) {
    exemptItem(h.name());
  }

  std::unique_ptr<TrackingContext> generateContext(mtl::GPhaseContext &ctx,
                                                   bool commitOnDelete = false);

  void writeTombstone(mtl::GPhaseContext &ctx,Nonce nonce);
  
  void accompanyWrite(mtl::GPhaseContext &, Name name, Nonce tombstone_value);

  void checkForTombstones(mtl::GPhaseContext &, Name name);

  void onCausalRead(mtl::GPhaseContext &pctx, Name name,
			     const Clock &version,
			     const std::function<void(char const *)> &construct_and_merge);
    
  // return is non-null when read value cannot be used.
  template <typename T>
  std::unique_ptr<T>
  onCausalRead(mtl::GPhaseContext &, Name name, const Clock &version,
         std::unique_ptr<T> candidate, 
         std::unique_ptr<T> (*merge)(char const *, std::unique_ptr<T>) =
             [](char const *, std::unique_ptr<T> r) { return r; });

  // for when merging locally is too hard or expensive
  bool waitForCausalRead(mtl::GPhaseContext &ctx, Name name,
                         const Clock &version);

  void afterCausalRead(TrackingContext &, Name name,
                       const Clock &version, const std::vector<char> &data);

  // for testing
  void assert_nonempty_tracking() const;
  const CooperativeCache &getcache() const;

  friend struct TrackingContext;

  Tracker(int cache_port, CacheBehaviors behavior /*= CacheBehaviors::full*/);
  virtual ~Tracker();

  Tracker(const Tracker &) = delete;

  const int cache_port;
};
}
}
