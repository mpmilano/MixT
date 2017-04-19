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
  using Nonce = tracker::Nonce;

  using StampedObject =
      mutils::TrivialTriple<Name, Tracker::Clock, std::vector<char>>;

  // hiding private members of this class. No implementation available.
  struct Internals;
  Internals *i;

  static constexpr int clockport = 9999;
  void updateClock();

  std::unique_ptr<TrackingContext> generateContext(mtl::TrackedPhaseContext &ctx);

  Nonce generateTombstone();
  
  void writeTombstone(mtl::TrackedPhaseContext &ctx,Nonce nonce);
  
  void accompanyWrite(mtl::TrackedPhaseContext &, Name name, Nonce tombstone_value);

  void checkForTombstones(mtl::TrackedPhaseContext &, Name name);

  void find_tombstones(mtl::TrackedPhaseContext &, const Tombstone&);

	void record_timestamp(mtl::TrackedPhaseContext &, const Clock& c);

  std::vector<Tombstone>& all_encountered_tombstones();
	void clear_encountered_tombstones();

  friend struct TrackingContext;

  Tracker();
  virtual ~Tracker();

  Tracker(const Tracker &) = delete;
};
}
}
