#pragma once
#include "parse_bindings.hpp"
#include "parse_expressions.hpp"
#include "parse_statements.hpp"
#include "flatten_expressions.hpp"
#include "label_inference.hpp"
#include "split_phase.hpp"
#include "interp.hpp"
#include "CTString.hpp"
#include "find_tombstones.hpp"
#include "recollapse.hpp"
#include <memory>
#include "Basics.hpp"
#include "simple_rpc.hpp"
#include "environments.hpp"
#include "runnable_transaction.hpp"
#include "run_phase.hpp"
#include "mtlbasics.hpp"
#include "myria_utils.hpp"
#include "local_connection.hpp"
#include "insert_tracking.hpp"

namespace myria {

namespace mtl {
namespace runnable_transaction {}
}

namespace server {

	template <txnID_t txnID, typename phase, typename tracked_phase, typename store>
struct transaction_listener;

	template <txnID_t txnID, typename l, typename returns, 
						typename AST, typename reqs, typename provides, typename owns, typename passthrough,
						txnID_t tracked_txnID, typename tracked_AST, typename tracked_reqs, typename tracked_provides, typename tracked_owns, typename tracked_passthrough,
						typename... holders>
struct transaction_listener<
    txnID, mtl::runnable_transaction::phase<txnID, l, returns, AST, reqs,
                                            provides, owns, passthrough>,
	mtl::runnable_transaction::phase<tracked_txnID, l, returns, tracked_AST, tracked_reqs,
																	 tracked_provides, tracked_owns, tracked_passthrough>,
    mtl::runnable_transaction::store<holders...>> {

  using label = l;
  using phase = mtl::runnable_transaction::phase<txnID, l, returns, AST, reqs,
                                                 provides, owns, passthrough>;
	using tracked_phase =	mtl::runnable_transaction::phase<tracked_txnID, l, returns, tracked_AST, tracked_reqs,
																												 tracked_provides, tracked_owns, tracked_passthrough>;
  using store = mtl::runnable_transaction::store<holders...>;

  template<typename DataStore>
  static bool run_if_match(std::size_t, txnID_t id,
			   DataStore &ds,
			   tracker::Tracker &trk,
                           mutils::DeserializationManager &dsm,
                           mutils::connection &c, char const *const _data) {
    using namespace mutils;
    if (id == txnID) {
			whendebug(std::string exn_text);
      auto tombstones_to_find =
          mutils::from_bytes_noalloc<std::vector<tracker::Tombstone>>(&dsm,
                                                                      _data);
      mutils::local_connection _lc;
      _lc.data = *mutils::from_bytes_noalloc<std::vector<char>>(
          &dsm, _data + mutils::bytes_size(*tombstones_to_find));
#ifndef NDEBUG
      mutils::connection &lc = _lc;
      auto &logfile = c.get_log_file();
      logfile << "receiving with store " << type_name<store>() << std::endl;
      logfile << "receiving id " << id << " for phase " << phase{};
      logfile.flush();
      std::size_t txn_nonce{0};
      lc.receive(txn_nonce);
      logfile << "transaction nonce: " << txn_nonce << std::endl;
      logfile.flush();
      c.send(txn_nonce);
#endif
      store s;
      constexpr reqs requires{};
      constexpr provides provided{};
      receive_store_values(&dsm, requires, s, _lc);
      bool transaction_successful{true};
      try {
				tracker::find_tombstones(ds,trk,dsm,*tombstones_to_find);
				mtl::runnable_transaction::common_interp<phase, store>(s,trk);
      } catch (std::exception &whendebug(e)) {
        // right now, *any* failure is just sent to the client as a byte;
        transaction_successful = false;
				whendebug(exn_text = e.what());
      }
      whendebug(logfile << "about to send response to client" << std::endl);
      if (transaction_successful) {
        mutils::local_connection lc;
        send_store_values(provided, s, lc);
				trk.updateClock();
        c.send(transaction_successful, trk.min_clock(), trk.recent_clock(), trk.all_encountered_tombstones(), lc.data);
      } else
        c.send(false whendebug(, mutils::bytes_size(exn_text), exn_text));
      whendebug(logfile << "response sent to client" << std::endl);
      return true;
    } else
      return false;
  }
};

template <typename transaction, typename phase>
using listener_for = transaction_listener<
	phase::txnID::value, phase,
	typename tombstone_enhanced_txn<typename transaction::previous_transaction_phases, typename phase::label>::template find_phase<typename phase::label>,
	typename transaction::all_store::template restrict_to_phase<phase>>;
}
}
