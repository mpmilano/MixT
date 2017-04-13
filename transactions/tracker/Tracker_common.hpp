#pragma once
#include "Tracker.hpp"
#include <tuple>

namespace myria {
namespace tracker {

template <typename T>
std::unique_ptr<T> Tracker::onCausalRead(
    mtl::TrackedPhaseContext &ctx, Name name, const Clock &version,
    std::unique_ptr<T> candidate,
    std::unique_ptr<T> (*merge)(char const *, std::unique_ptr<T>)) {

  struct Owner {
    std::unique_ptr<T> candidate;
    std::unique_ptr<T> merged{nullptr};
    Owner(decltype(candidate) &candidate) : candidate(std::move(candidate)) {}
  };
  Owner mem{candidate};

  std::function<void(char const *)> c_merge{[merge, &mem](char const *arg) {
    assert(mem.candidate);
    assert(!mem.merged);
    mem.merged = merge((char const *)arg, std::move(mem.candidate));
  }};

  onCausalRead(ctx, name, version, c_merge);
  if (mem.merged)
    return std::move(mem.merged);
  else
    return std::move(mem.candidate);
}
}
}
