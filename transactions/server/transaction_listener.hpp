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
#include "environments.hpp"
#include "runnable_transaction.hpp"
#include "run_phase.hpp"
#include "mtlbasics.hpp"
#include "myria_utils.hpp"

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
			
			static bool run_if_match(txnID_t id, mutils::DeserializationManager& dsm, mutils::connection &c, 
															 char const * const _data){
				using namespace mutils;
				if (id == txnID){
					std::size_t request_size = ((std::size_t*)_data)[0];
					auto* data = _data + sizeof(request_size);
					std::unique_ptr<ClientRequestMessage<store> > msg =
						ClientRequestMessage<store>::from_bytes(&dsm,data);
					ServerReplyMessage<Name,store> srm{{},std::move(msg->store)};
					
					mtl::runnable_transaction::common_interp<phase, store>(*srm.store);
					/*
					const bool b = _store_diff->complies(*srm.store);
					if(!b){
						assert(false && "populate the cache somehow! Do stuff!");
						}*/
					c.send(srm.bytes_size());
					c.send(srm);
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
