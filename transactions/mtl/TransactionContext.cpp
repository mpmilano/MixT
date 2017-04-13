#include "TransactionContext.hpp"
#include "Tracker.hpp"
namespace myria {
  namespace mtl {
    
    TrackedPhaseContext::TrackedPhaseContext(tracker::Tracker &trk)
      :trk_ctx(trk,*this,false){}
}
}
