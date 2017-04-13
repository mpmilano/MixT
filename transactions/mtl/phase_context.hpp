#pragma once

namespace myria {
  namespace mtl {
    template <typename label>
    _PhaseContext<Label<label>, true>::_PhaseContext(tracker::Tracker &trk)
      : TrackedPhaseContext(trk) {}
}
}
