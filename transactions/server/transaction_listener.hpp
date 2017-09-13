#pragma once
#include "mtl/parse_bindings.hpp"
#include "mtl/parse_expressions.hpp"
#include "mtl/parse_statements.hpp"
#include "mtl/flatten_expressions.hpp"
#include "mtl/label_inference.hpp"
#include "mtl/split_phase.hpp"
#include "mtl/interp.hpp"
#include "mutils/CTString.hpp"
#include "tracker/find_tombstones.hpp"
#include "mtl/recollapse.hpp"
#include <memory>
#include "Basics.hpp"
#include "mutils-networking/simple_rpc.hpp"
#include "mtl/environments.hpp"
#include "mtl/runnable_transaction.hpp"
#include "mtl/run_phase.hpp"
#include "mtl/mtlbasics.hpp"
#include "myria-utils/myria_utils.hpp"
#include "mutils-networking/local_connection.hpp"
#include "mtl/insert_tracking.hpp"

namespace myria {

namespace mtl {
namespace runnable_transaction {}
}

namespace server {

	template <typename phase, typename normal_store, typename tracked_phase, typename tracked_store>
struct transaction_listener;

	template<typename phase,typename store, typename DataStore>
	static bool run_phase(txnID_t whendebug(id), DataStore &ds,
												tracker::Tracker &trk,
												mutils::DeserializationManager &dsm,
												mutils::connection &c, char const *const _data){
		using reqs = typename phase::requirements;
		using provides = typename phase::provides;
		using namespace mutils;
		whendebug(std::string exn_text);
		auto tombstones_to_find =
			mutils::from_bytes_noalloc<std::vector<tracker::Tombstone>>(&dsm,
																																	_data);
		mutils::local_connection _lc whendebug({c.get_log_file()});
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
		DECT(mutils::bytes_size(std::string{})) phase_str_size;
		lc.receive(phase_str_size);
		std::string remote_phase_str = *lc.template receive<std::string>(nullptr,phase_str_size);
		logfile << "remote phase str: " << remote_phase_str << std::endl;
		{
			std::stringstream ss;
			ss << phase{};
			std::string phase_str = ss.str();
			if (remote_phase_str != phase_str){
				std::cout << remote_phase_str << std::endl;
				std::cout << phase_str << std::endl;
			}
			auto* remote_phase_charstar = remote_phase_str.c_str();
			auto* phase_charstar = phase_str.c_str();
			(void) remote_phase_charstar;
			(void) phase_charstar;
			assert(remote_phase_str == phase_str);
		}
		logfile << "transaction nonce: " << txn_nonce << std::endl;
		logfile.flush();
		c.send(txn_nonce);
#endif
		store s;
		constexpr reqs requires{};
		constexpr provides provided{};
		whendebug(s.get(requires));
		whendebug(s.get(provided));
		receive_store_values(&dsm, requires, s, _lc);
		bool transaction_successful{true};
		try {
			tracker::find_tombstones(ds,trk,dsm,*tombstones_to_find);
			trk.set_persistent_store(ds);
			mtl::runnable_transaction::common_interp<phase, store>(s,trk);
		} catch (std::exception &whendebug(e)) {
			// right now, *any* failure is just sent to the client as a byte;
			transaction_successful = false;
			whendebug(exn_text = e.what());
		}
		whendebug(logfile << "about to send response to client" << std::endl);
		if (transaction_successful) {
		  mutils::local_connection lc whendebug({logfile});
			send_store_values(provided, s, lc);
			trk.updateClock();
			c.send(transaction_successful, trk.min_clock(), trk.recent_clock(), trk.all_encountered_tombstones(), lc.data);
		} else
			c.send(false whendebug(, mutils::bytes_size(exn_text), exn_text));
		whendebug(logfile << "response sent to client" << std::endl);
		return true;
	}

	template <typename l, typename returns, 
						typename AST, typename reqs, typename provides, typename owns, typename passthrough,
						typename tracked_AST, typename tracked_reqs, typename tracked_provides, typename tracked_owns, typename tracked_passthrough,
						typename _tracked_store,
						typename... normal_holders>
struct transaction_listener<
    mtl::runnable_transaction::phase<l, returns, AST, reqs,
                                            provides, owns, passthrough>,
		mtl::runnable_transaction::store<normal_holders...>,
		mtl::runnable_transaction::phase<l, returns, tracked_AST, tracked_reqs,
																	 tracked_provides, tracked_owns, tracked_passthrough>,
		_tracked_store
    > {

  using label = l;
  using normal_phase = mtl::runnable_transaction::phase<l, returns, AST, reqs,
                                                 provides, owns, passthrough>;
		using normal_store = mtl::runnable_transaction::store<normal_holders...>;
	using tracked_phase =	mtl::runnable_transaction::phase<l, returns, tracked_AST, tracked_reqs,
																												 tracked_provides, tracked_owns, tracked_passthrough>;
		
		using tracked_store = _tracked_store;
	
  template<typename DataStore>
  static bool run_if_match(std::size_t, txnID_t id,
			   DataStore &ds,
			   tracker::Tracker &trk,
                           mutils::DeserializationManager &dsm,
                           mutils::connection &c, char const *const _data) {
    using namespace mutils;
    if (id == normal_phase::txnID()) return run_phase<normal_phase, normal_store>(id,ds,trk,dsm,c,_data);
		else if (id == tracked_phase::txnID()) return run_phase<tracked_phase, tracked_store>(id,ds,trk,dsm,c,_data);
		else return false;
  }
};

template <typename transaction, typename phase>
using listener_for = transaction_listener<
	phase,typename transaction::all_store::template restrict_to_phase<phase>,
	typename tombstone_enhanced_txn<typename transaction::previous_transaction_phases, typename phase::label>::template find_phase<typename phase::label>,
	tombstone_enhanced_store<typename transaction::previous_transaction_phases, typename phase::label>
	>;
}
}
