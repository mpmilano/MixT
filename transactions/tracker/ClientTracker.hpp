#pragma once
#include "Ends.hpp"
#include "Tracker.hpp"
#include "Tombstone.hpp"

namespace myria {
namespace tracker {

template <typename label> struct TombHolder;

template <typename label> struct TombHolder<Label<label>> {
  std::unique_ptr<std::vector<Tombstone>> obligations{
      new std::vector<Tombstone>()};
	std::set<Tombstone> observed_tombstones;
	Clock global_min_clock;
	Clock max_recent_clock;
	using connection = mutils::connection;
	connection& store_connection;
	TombHolder(connection& c):store_connection(c){}
	
	void reset_obligations(){
		obligations.reset(new std::vector<Tombstone>());
	}

	bool must_track() const {
		return !(max_recent_clock < global_min_clock || max_recent_clock == global_min_clock);
	}

protected:
  ~TombHolder() = default;
};

template <typename... labels>
struct ClientTracker : public TombHolder<labels>... {
  Tracker local_tracker;

	ClientTracker(typename TombHolder<labels>::connection & ... connections)
		:TombHolder<labels>(connections)...{}
	
  template <typename phase> std::vector<Tombstone> &tombstones_for_phase() {
    return *TombHolder<typename phase::label>::obligations;
  }

  template <typename phase> void clear_tombstones() {
    using TH = TombHolder<typename phase::label>;
		TH::reset_obligations();
  }
	
	bool must_track() const {
		return (false || ... || TombHolder<labels>::must_track());
	}

	template<typename phase> void update_clocks(const Clock& global_min_clock, const Clock &recent_clock){
		using TH = TombHolder<typename phase::label>;
		TH::global_min_clock = ends::max(global_min_clock, TH::global_min_clock);
		TH::max_recent_clock = ends::max(TH::max_recent_clock, recent_clock);
	}

  template <typename phase>
  void set_phase_after(std::unique_ptr<std::vector<Tombstone>> ptr) {
		using NextHolder =
			TombHolder<mutils::follows_in_sequence<typename phase::label,labels...>>;
		NextHolder::reset_obligations();
		for (const auto &tomb : *ptr){
			if (!NextHolder::observed_tombstones.count(tomb)){
				NextHolder::obligations->push_back(tomb);
				NextHolder::observed_tombstones.insert(tomb);
			}
		}
  }

	template<typename previous_transaction_phases>
		using alternative_tracked_txn = tombstone_enhanced_txn<previous_transaction_phases, labels...>;
	
};
}
}
