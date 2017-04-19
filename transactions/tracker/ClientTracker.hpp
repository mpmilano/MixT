#pragma once
#include "Tracker.hpp"

namespace myria {
namespace tracker {

template <typename label> struct TombHolder;

template <typename label> struct TombHolder<Label<label>> {
  std::unique_ptr<std::vector<tracker::Tombstone>> obligations{
      new std::vector<tracker::Tombstone>()};
  std::unique_ptr<std::vector<tracker::Tombstone>> fulfilled{
      new std::vector<tracker::Tombstone>()};

protected:
  ~TombHolder() = default;
};

template <typename... labels>
struct ClientTracker : public TombHolder<labels>... {
  Tracker local_tracker;

  template <typename phase> std::vector<Tombstone> &tombstones_for_phase() {
    return *TombHolder<typename phase::label>::obligations;
  }

  template <typename phase> std::vector<Tombstone> &fulfilled_for_phase() {
    return *TombHolder<typename phase::label>::fulfilled;
  }

  template <typename phase> void mark_tombstones_clearable() {
    using TH = TombHolder<typename phase::label>;
    TH::fulfilled = std::move(TH::obligations);
    TH::obligations.reset(new std::vector<tracker::Tombstone>());
  }

  template <typename phase>
  void set_phase_after(std::unique_ptr<std::vector<tracker::Tombstone>> ptr) {
    TombHolder<mutils::follows_in_sequence<typename phase::label,
                                           labels...>>::obligations =
        std::move(ptr);
  }
};
}
}
