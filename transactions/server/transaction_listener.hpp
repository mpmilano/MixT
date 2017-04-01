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
#include "batched_connection.hpp"


namespace myria { namespace server{

		using txnID_t = unsigned short;
		
		template<txnID_t txnID, typename phase, typename store>
		struct transaction_listener {

			using label = typename phase::label;
			
			static bool run_if_match(txnID_t id, DeserializationManager& dsm, mutils::connection &c, 
															 char const * const data){
				if (id == txnID){
					std::context_ptr<ClientRequestMessage<store> > msg =
						ClientRequestMessage<store>::from_bytes(dsm,data);
					ServerReplyMessage srm{{},std::move(msg->store)};
					
					mtl::runnable_transaction::common_interp<phase, store>(*srm.store);
					const bool b = _store_diff->complies(*srm.store);
					if(!b){
						assert(false && "populate the cache somehow! Do stuff!");
					}
					c.send(srm);
					return true;
				}
				else return false;
			}
		};
	}}
