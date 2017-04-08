#pragma once
#include "AST_split.hpp"
#include "split_printer.hpp"
#include "environments.hpp"
#include "runnable_transaction.hpp"
#include "run_phase.hpp"
#include "remote_interp.hpp"
#include "Basics.hpp"
#include "local_connection.hpp"

namespace myria { namespace mtl { namespace runnable_transaction {

			template<typename phase, typename FullStore, typename tombstone_tracker>
			auto remote_interp(mutils::DeserializationManager* dsm, tombstone_tracker &trk, mutils::connection &c, FullStore &s){
				using namespace mutils;
				constexpr typename phase::requirements requires{};
				constexpr typename phase::provides provides{};
				mutils::local_connection lc;
#ifndef NDEBUG
				auto &logfile = c.get_log_file();;
				std::size_t txn_nonce{mutils::int_rand()};
				logfile << "sending id " <<
					phase::txnID::value << " with nonce " << txn_nonce << " for phase " << phase{} << std::endl;
				mutils::connection &mlc = lc;
				mlc.send(txn_nonce);
#endif
				send_store_values(requires,s,lc);
				c.send(phase::txnID::value, trk.template tombstones_for_phase<phase>(), lc.data);
#ifndef NDEBUG
				std::size_t remote_txn_nonce;
				c.receive(remote_txn_nonce);
				assert(remote_txn_nonce == txn_nonce);
#endif
				{
					mutils::local_connection lc;
					auto new_tombstones = mutils::receive_from_connection<std::vector<Tombstone> >(dsm,c);
					lc.data = *mutils::receive_from_connection<std::vector<char> >(dsm,c);
					receive_store_values(dsm,provides,s,lc);
					trk.template clear_tombstones_for_phase<phase>();
				}
			}
			
		}}}
