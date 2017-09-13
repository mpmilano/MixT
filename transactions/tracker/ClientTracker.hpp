#pragma once
#include "tracker/Ends.hpp"
#include "tracker/Tracker.hpp"
#include "tracker/Tombstone.hpp"
#include "mtl/insert_tracking.hpp"
#include "mtl/mtlbasics.hpp"

namespace myria {
namespace tracker {

template <typename label> struct TombHolder;

template <typename label> struct TombHolder<Label<label>> {
  std::unique_ptr<std::vector<Tombstone>> obligations{
      new std::vector<Tombstone>()};
	std::set<Tombstone> observed_tombstones;
	Clock global_min_clock;
	Clock max_recent_clock;
	void reset_obligations(){
		obligations.reset(new std::vector<Tombstone>());
	}

	bool must_track() const {
		return !(max_recent_clock < global_min_clock || max_recent_clock == global_min_clock);
	}

protected:
  ~TombHolder() = default;
};

	template<typename label> struct ConnectionReference{
		using connection = mutils::connection;
		mutils::connection & c;
		ConnectionReference(DECT(c) & c):c(c){
#ifndef NDEBUG
			constexpr txnID_t debug_txn{0};
			c.send(debug_txn);
			DECT(mutils::bytes_size(std::string{})) rcv_size{0};
			c.receive(rcv_size);
			assert(*c.template receive<std::string>(nullptr,rcv_size) == std::string{typename label::description{}.string});
#endif
		}
	};

	template<typename... label> struct ConnectionReferences : public ConnectionReference<label>...{
		ConnectionReferences(typename ConnectionReference<label>::connection&... c)
			:ConnectionReference<label>(c)...{}

		ConnectionReferences(const ConnectionReference<label>&... c)
			:ConnectionReference<label>(c.c)...{}

		template<typename phase> auto& connection(){
			using CR = ConnectionReference<typename phase::label>;
			CR& _this = *this;
			return _this;
		}
	};

template <typename... labels>
struct _ClientTracker : public TombHolder<labels>... {
  Tracker local_tracker;

	using connection_references = ConnectionReferences<labels...>;
	
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
		void set_phase_after(std::unique_ptr<std::vector<Tombstone>> ptr,
												 std::enable_if_t<!mutils::is_sequence_end<typename phase::label,labels...>::value>* = nullptr) {
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

	template <typename phase>
	void set_phase_after(std::unique_ptr<std::vector<Tombstone>> ,
											 std::enable_if_t<mutils::is_sequence_end<typename phase::label,labels...>::value>* = nullptr) {}

	template<typename previous_transaction_phases>
		using alternative_tracked_txn = tombstone_enhanced_txn<previous_transaction_phases, labels...>;
	
	template<typename previous_transaction_phases>
		using alternative_tracked_store = tombstone_enhanced_store<previous_transaction_phases, labels...>;
	
};

	template<typename> struct ClientTracker_from_typeset;
	template<typename... labels> struct ClientTracker_from_typeset<mutils::typeset<labels...> >{
		using type = _ClientTracker<labels...>;
	};
	
	template<typename... labels> using ClientTracker =
		typename ClientTracker_from_typeset<DECT(
		mtl::typecheck_phase::tracking_phase::tracked_labels(mutils::typelist<labels...>{}).combined()) >::type;
}
}
