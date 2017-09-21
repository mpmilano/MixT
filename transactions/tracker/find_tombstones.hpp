#pragma once
#include "Tracker.hpp"

namespace myria { namespace tracker {
    template<typename DataStore, typename... ctxs>
    auto find_tombstones(DataStore& ds,tracker::Tracker& trk,mutils::DeserializationManager<ctxs...>& ,const std::vector<tracker::Tombstone> &tombstones_to_find){
      using namespace mtl;
      PhaseContext<typename DataStore::label> ctx{trk};
      ctx.store_context(ds whendebug(, "hunting tombstones"));
      for (const auto& tomb : tombstones_to_find){
				trk.find_tombstones(ctx,tomb);
      }
    }
  }}
