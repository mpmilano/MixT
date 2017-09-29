#pragma once
#include "mtl/AST_split.hpp"
#include "mtl/split_printer.hpp"
#include "mtl/environments.hpp"
#include "mtl/runnable_transaction.hpp"
#include "mtl/run_phase.hpp"
#include "mtl/remote_interp.hpp"
#include "tracker/ClientTracker.hpp"
#include "Basics.hpp"
#include "mutils-networking/local_connection.hpp"

namespace myria {
namespace mtl {
namespace runnable_transaction {

	template <typename phase, typename FullStore, typename tombstone_tracker, typename... ctxs>
	auto remote_interp(mutils::DeserializationManager<ctxs...>* dsm, tombstone_tracker& trk, tracker::ConnectionReference<typename phase::label>& c, FullStore& s)
{
  using namespace mutils;
  constexpr typename phase::requirements requires{};
  constexpr typename phase::provides provides{};
  mutils::local_connection lc whendebug({c.c.get_log_file()});
	//NOTE TO SELF: USE THE TOMBSTONE TRACKER TO FIND ALL THE LEVELS WE CARE ABOUT.
	//CALL THE JUST-WRITE-A-TOMBSTONE TRANSACTION FROM THERE TOO.
#ifndef NDEBUG
	{
		static bool b = false;
		static std::mutex print_mut;
		if (!b){
			std::unique_lock<DECT(print_mut)> l{print_mut};
			if (!b){
				std::cout << phase{} << std::endl;
				b = true;
			}
		}
	}
  auto& logfile = c.c.get_log_file();
  ;
  std::size_t txn_nonce{ mutils::int_rand() };
  logfile << "sending id " << phase::txnID() << " with nonce " << txn_nonce << " for phase " << phase{} << std::endl;
  mutils::connection& mlc = lc;
#ifndef NDEBUG
	std::string phase_str;
	{
		std::stringstream ss;
		ss << phase{};
		phase_str = ss.str();
	}
#endif
	mlc.send(txn_nonce,mutils::bytes_size(phase_str));
	whendebug(auto phase_str_starts_at = lc.data.size());
	whendebug(auto sent =) mlc.send(phase_str);
	assert(mutils::bytes_size(phase_str) >= phase_str.length());
	logfile << "sending phase str: " << phase_str << std::endl;
	assert(sent == mutils::bytes_size(phase_str));
  assert(lc.data.size() >= sizeof(txn_nonce));
#endif
  send_store_values<typename phase::label>(requires, s, lc);
	whendebug(char* data = lc.data.data());
	assert(phase_str == data + phase_str_starts_at);
  c.c.send(phase::txnID(),
	 trk.template tombstones_for_phase<phase>(),
	 lc.data);
#ifndef NDEBUG
  std::size_t remote_txn_nonce;
  c.c.receive(remote_txn_nonce);
  assert(remote_txn_nonce == txn_nonce);
#endif
  {
    mutils::local_connection lc whendebug({logfile});
    if (*mutils::receive_from_connection<bool>(dsm, c.c)) {
      // transaction was successful!
			{
				auto new_min_clock = mutils::receive_from_connection<tracker::Clock>(dsm,c.c);
				auto new_recent_clock = mutils::receive_from_connection<tracker::Clock>(dsm,c.c);
				auto new_tombstones = mutils::receive_from_connection<std::vector<tracker::Tombstone>>(dsm, c.c);
				trk.template update_clocks<phase>(*new_min_clock, *new_recent_clock);
				trk.template set_phase_after<phase>(std::move(new_tombstones));
			}
      lc.data = *mutils::receive_from_connection<std::vector<char>>(dsm, c.c);
      receive_store_values<typename phase::label>(dsm, provides, s, lc);
      trk.template clear_tombstones<phase>();
    } else {
			std::string failure_str = "";
#ifndef NDEBUG
			DECT(mutils::bytes_size(std::string{})) size{0};
			c.c.receive(size);
			failure_str = *c.c.template receive<std::string>(dsm,size);
			if (failure_str.size() > 24 && failure_str[24] == 'S'){
				std::cout << failure_str << std::endl;
				assert(false);
			}
#endif
      // store aborted, or crashed, or had a bad day
      throw SerializationFailure{failure_str};
    }
  }
}
}
}
}
