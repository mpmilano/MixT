#pragma once

namespace myria {
namespace mtl {
  struct TrackedPhaseContext;
}
}

namespace myria {
namespace tracker {

class Tracker;

struct TrackingContext {
  struct Internals;
  Internals *i{nullptr};
  Tracker &trk;
  mtl::TrackedPhaseContext &ctx;
  TrackingContext(Tracker &t, mtl::TrackedPhaseContext &ctx);
  void commitContext();
  virtual ~TrackingContext();
  TrackingContext(const TrackingContext &) = delete;
  TrackingContext(TrackingContext &&) = delete;
  friend class Tracker;
};
}
}
