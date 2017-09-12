#include "tracker/Tracker.hpp"
#include "tracker/Tracker_private_declarations.hpp"
#include "mutils-networking/ServerSocket.hpp"
#include <thread>

using namespace std;
using namespace mutils;

namespace myria {
namespace tracker {

struct ClockManager {
  static ClockManager &inst() {
    static ClockManager inst;
    return inst;
  }

  volatile
      typename Tracker::Clock::value_type clock[Tracker::Clock{}.max_size()];

  ClockManager(const ClockManager &) = delete;

private:
  AcceptConnectionLoop loop{[&](bool &, Socket sock) {
    try {
      while (sock.valid()) {
        Tracker::Clock tmpclock;
        sock.receive(tmpclock);
        for (std::decay_t<decltype(tmpclock.size())> i = 0; i < tmpclock.size();
             ++i) {
          clock[i] = tmpclock[i];
        }
      }
    } catch (const ProtocolException &) {
    }
  }};

  ClockManager() {
    std::thread{[loop = this->loop]() mutable {
        try {loop.loop_until_dead(Tracker::clockport);
  }
  catch (mutils::SocketException &) {
    // looks like we're flying without the clock today.
  }
}
}
.detach();
}
~ClockManager() { *loop.alive = false; }
}
;

	namespace {
		auto *global_manager_init = &ClockManager::inst();
	}

void Tracker::updateClock() {
  Tracker::Clock newc;
  for (std::decay_t<decltype(newc.size())> i = 0; i < newc.size(); ++i) {
    auto &inst = ClockManager::inst();
    if (inst.clock[i] == -1) {
      --i;
      continue;
    }
    volatile long long &tmpi = inst.clock[i];
    newc[i] = tmpi;
  }
  assert(ends::prec(i->global_min, newc));
  i->global_min = newc;
}

	const Clock& Tracker::min_clock() const {
		return i->global_min;
	}

	const Clock& Tracker::recent_clock() const {
		return i->new_objects_max;
	}
	
}
}
