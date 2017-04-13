#pragma once
#include "Tracker.hpp"

namespace myria { namespace tracker {
    template<typename DataStore>
    tracker::find_tombstones(DataStore& ds,tracker::Tracker& trk,DeserializationManager& dsm,const std::vector &tombstones_to_find){
      PhaseContext<typename DataStore::label> ctx{trk};
      ctx.store_context(ds whendebug(, hunting tombstones));
      for (const auto& tomb : tombstones_for_phase){
	trk.checkForTombstones(ctx,tomb);
      }
    }
  }}
