#pragma once
#include "AST_split.hpp"
#include "split_printer.hpp"
#include "environments.hpp"
#include "runnable_transaction.hpp"
#include "run_phase.hpp"
#include "remote_interp.hpp"
#include "ClientTracker.hpp"
#include "Basics.hpp"
#include "local_connection.hpp"

namespace myria {
namespace mtl {
namespace runnable_transaction {

template <typename phase, typename FullStore, typename tombstone_tracker>
auto remote_interp(mutils::DeserializationManager* dsm, tombstone_tracker& trk, tracker::ConnectionReference<typename phase::label>& c, FullStore& s)
{
  using namespace mutils;
  constexpr typename phase::requirements requires{};
  constexpr typename phase::provides provides{};
  mutils::local_connection lc;
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
  mlc.send(txn_nonce);
  assert(lc.data.size() >= sizeof(txn_nonce));
#endif
  send_store_values(requires, s, lc);
  c.c.send(phase::txnID(),
	 trk.template tombstones_for_phase<phase>(),
	 lc.data);
#ifndef NDEBUG
  std::size_t remote_txn_nonce;
  c.c.receive(remote_txn_nonce);
  assert(remote_txn_nonce == txn_nonce);
#endif
  {
    mutils::local_connection lc;
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
      receive_store_values(dsm, provides, s, lc);
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
