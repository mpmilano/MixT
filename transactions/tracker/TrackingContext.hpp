#pragma once

namespace myria {
namespace mtl {
  struct GPhaseContext;
}
}

namespace myria {
namespace tracker {

class Tracker;

struct TrackingContext {
  struct Internals;
  Internals *i{nullptr};
  Tracker &trk;
  mtl::GPhaseContext &ctx;
  TrackingContext(Tracker &t, mtl::GPhaseContext &ctx, bool commitOnDelete = false);
  void commitContext();
  void abortContext();
  virtual ~TrackingContext();
  TrackingContext(const TrackingContext &) = delete;
  TrackingContext(TrackingContext &&) = delete;
  friend class Tracker;
};
}
}
