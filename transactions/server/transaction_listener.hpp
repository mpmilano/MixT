#pragma once
#include "parse_bindings.hpp"
#include "parse_expressions.hpp"
#include "parse_statements.hpp"
#include "flatten_expressions.hpp"
#include "label_inference.hpp"
#include "split_phase.hpp"
#include "interp.hpp"
#include "CTString.hpp"
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

namespace myria {

	namespace mtl {namespace runnable_transaction {
			


		}}
	
	namespace server{

		template<txnID_t txnID, typename phase, typename store>
		struct transaction_listener;
		
		template<txnID_t txnID,
						 typename l, typename AST, typename reqs, typename provides, typename owns, typename passthrough,
						 typename... holders>
		struct transaction_listener<txnID,
																mtl::runnable_transaction::phase<txnID, l,AST,reqs,provides,owns,passthrough>,
																mtl::runnable_transaction::store<holders...> > {

			using label = l;
			using phase = mtl::runnable_transaction::phase<txnID, l,AST,reqs,provides,owns,passthrough>;
			using store = mtl::runnable_transaction::store<holders...>;
			
			static bool run_if_match(std::size_t, txnID_t id, mutils::DeserializationManager& dsm, mutils::connection &c, 
															 char const * const _data){
				using namespace mutils;
				if (id == txnID){
				  	auto tombstones_to_find = mutils::from_bytes_noalloc<std::vector<Tombstone> >(&dsm,_data);
					mutils::local_connection _lc;
					_lc.data = *mutils::from_bytes_noalloc<std::vector<char> >(&dsm,_data);
					mutils::connection &lc = _lc;
#ifndef NDEBUG
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
					receive_store_values(&dsm,requires,s,_lc);
					mtl::runnable_transaction::common_interp<phase, store>(s);
					whendebug(logfile << "about to send response to client" << std::endl);
					{
						std::vector<Tombstone> encountered_tombstones;
						c.send(encountered_tombstones);
						mutils::local_connection lc;
						send_store_values(provided,s,lc);
						c.send(lc.data);
					}
					whendebug(logfile << "response sent to client" << std::endl);
					return true;
				}
				else return false;
			}
		};

		template<typename transaction, typename phase>
		using listener_for = transaction_listener<
			phase::txnID::value,
			phase,
			typename transaction::all_store::template restrict_to_phase<phase> >;
	}}
