#include "DataStore.hpp"
#include <cstdlib>
#include <time.h>
#include "GDataStore.hpp"
#include "compile-time-lambda.hpp"
#include "utils.hpp"
#include <functional>
#include <array>
#include <map>
#include <list>
#include <set>
#include <chrono>
#include <thread>
#include <unistd.h>

#include "CompactSet.hpp"
#include "Ends.hpp"
#include "SafeSet.hpp"
#include "Ostreams.hpp"
#include "Tracker_private_declarations.hpp"
namespace {
constexpr unsigned long long bigprime_lin =
#include "big_prime"
    ;
}
#include <cstdlib>

namespace myria {
namespace tracker {

using namespace std;
using namespace chrono;
using namespace mutils;
using namespace mtl;
using namespace tracker;

void TrackingContext::commitContext() {
  i->_commitContext();
}
	
TrackingContext::TrackingContext(Tracker &trk, TrackedPhaseContext &ctx)
  : i(new TrackingContext::Internals{trk,*trk.i}), trk(trk), ctx(ctx) {}

TrackingContext::~TrackingContext() {
  if (i)
    delete i;
}

std::unique_ptr<TrackingContext> Tracker::generateContext(TrackedPhaseContext &ctx) {
  return std::unique_ptr<TrackingContext>{
      (new TrackingContext{*this, ctx})};
}

Tracker::Tracker(): i{new Internals{}} {}

Tracker::~Tracker() { delete i; }

Name Tracker::Tombstone::name() const { return nonce; }

namespace {

// constexpr int bigprime_causal = 2751103;

bool is_metaname(long int base, Name name) {
  return (name > 0) && ((name % base) == 0);
}

Name make_metaname(long int base, Name name) {
  assert([=]() {
    Name sanity = numeric_limits<int>::max() * base;
    return sanity > 0;
  }());
  assert(name <= numeric_limits<int>::max());
  assert(name > 0);
  assert(!is_metaname(base, name));
  Name cand = name * base;
  assert(cand > 0);
  assert(is_metaname(base, cand));
  return cand;
}

bool is_lin_metadata(Name name) { return is_metaname(bigprime_lin, name); }

Name make_lin_metaname(Name name) { return make_metaname(bigprime_lin, name); }

int get_ip() {
  static int ip_addr{[]() {
    std::string static_addr{MY_IP};
    if (static_addr.length() == 0)
      static_addr = "127.0.0.1";
    return (int)mutils::decode_ip(static_addr.c_str());
  }()};
  return ip_addr;
}
}
  struct TombNameCollision : public SerializationFailure {
    TombNameCollision():SerializationFailure("tomb name collision"){}
  };

  Tombstone Tracker::generateTombstone(){ return Tombstone{long_rand(),get_ip(),0}; }

void Tracker::writeTombstone(mtl::TrackedPhaseContext &ctx,Tracker::Nonce nonce) {
  const Tracker::Tombstone t{nonce, get_ip(), 0};
  assert(ctx.store_context());
  TrackableDataStore_super &ds =
    dynamic_cast<TrackableDataStore_super &>(ctx.store_context()->store());
  if (ds.exists(&ctx, t.name()))
    throw TombNameCollision{};
  else
    ds.new_tomb(&ctx, t.name(), t);
}

  void Tracker::accompanyWrite(mtl::TrackedPhaseContext &ctx, Name name, Nonce nonce) {
  const auto write_lin_metadata = [this](mtl::TrackedPhaseContext &ctx,
                                         StrongTrackableDataStore &ds_real,
                                         Name name, Tracker::Nonce nonce) {
    assert(ctx.store_context());
    auto meta_name = make_lin_metaname(name);
    if (ds_real.exists(&ctx, meta_name)) {
      ds_real.existing_tombstone(&ctx, meta_name)
          ->put(&ctx, Tracker::Tombstone{nonce, get_ip(), 0});
    } else {
      ds_real.new_tomb(&ctx, meta_name,
                       Tracker::Tombstone{nonce, get_ip(), 0});
    }
  };

  assert(ctx.store_context());
  auto &ds_real =
      dynamic_cast<StrongTrackableDataStore &>(ctx.store_context()->store());
  if (!is_lin_metadata(name)) {

    auto subroutine = [&]() {
      write_lin_metadata(ctx, ds_real, name, nonce);
    };
    bool always_failed = true;
    auto sleep_time = 2ms;
    for (int asdf = 0; asdf < 10; ++asdf) {
      try {
        subroutine();
        always_failed = false;
        break;
      } catch (const TombNameCollision &) {
        this_thread::sleep_for(sleep_time);
        sleep_time *= 2;
        // assume we picked a bad nonce, try again
      }
    }
    if (always_failed) {
      // it's almost certainly going to fail again, but at this point
      // we are really interested in what the error is.
      subroutine();
    }
    assert(!always_failed);
    assert(ds_real.exists(&ctx, make_lin_metaname(name)));
  }
}

std::ostream &operator<<(std::ostream &os, const Tracker::Clock &c) {
  os << "Clock: [";
  for (auto &a : c) {
    os << a << ",";
  }
  return os << "]";
}

  void Tracker::find_tombstones(mtl::TrackedPhaseContext &ctx, const Tombstone& t){
    TrackableDataStore_super &ds =
      dynamic_cast<TrackableDataStore_super &>(ctx.store_context()->store());
		auto sleep_for = 1ms;
		constexpr auto max_sleep_for = 1min;
    while (!ds.exists(&ctx,t.name())) {
			this_thread::sleep_for(max_sleep_for > sleep_for ? sleep_for : max_sleep_for);
			sleep_for *= 2;
    }
  }

	void Tracker::record_timestamp(mtl::TrackedPhaseContext & ctx, const Clock &s){
		ctx.trk_ctx.i->newer_objects.push_back(s);
	}

  std::vector<Tombstone>& Tracker::all_encountered_tombstones(){
		assert(i->encountered_tombstones);
		return *i->encountered_tombstones;
  }

	void Tracker::clear_encountered_tombstones(){
		i->encountered_tombstones.reset(new std::vector<Tombstone>{});
	}

  void Tracker::checkForTombstones(mtl::TrackedPhaseContext &sctx, Name name){
    TrackingContext &tctx = sctx.trk_ctx;
    assert(name != 1);
    assert(sctx.store_context());
    TrackableDataStore_super &ds =
      dynamic_cast<TrackableDataStore_super &>(sctx.store_context()->store());
    
    if (!is_lin_metadata(name)) {
      auto ts = make_lin_metaname(name);
      if (ds.exists(&sctx, ts)) {
	auto tomb_p = ds.existing_tombstone(&sctx, ts)->get(&sctx);
	auto &tomb = *tomb_p;
	// std::cout << "Nonce isn't immediately available, adding to
	// pending_nonces" << std::endl;
	tctx.i->pending_nonces->emplace_back(tomb);
      }
    }
  }
}
}
