#pragma once
#include "AST_split.hpp"
#include "split_printer.hpp"
#include "environments.hpp"
#include "runnable_transaction.hpp"
#include "run_phase.hpp"
#include "remote_interp.hpp"
#include "ServerReplyMessage.hpp"
#include "Basics.hpp"

namespace myria { namespace mtl { namespace runnable_transaction {


			template<typename store, char... str>
			void send_holder_values(mutils::String<str...> holder_name, store &s, mutils::connection &c){
				using holder = typename store::find_holder_by_name<DECT(holder_name)>;
				holder& h = s;
				
			}
			
			template<typename store, typename... requires>
			void send_store_values(const mutils::typeset<requires...>&, store &s, mutils::connection &c){
			}
			

			template<typename phase, typename FullStore>
			auto remote_interp(mutils::DeserializationManager* dsm, mutils::connection &c, FullStore &s){
				using namespace mutils;
				using namespace server;
				whendebug(auto &logfile = c.get_log_file(););
				using restricted_store
					= typename FullStore::template restrict_to_phase<phase>;
				whendebug(logfile << "sending with store " << type_name<restricted_store>() << std::endl);
				ClientRequestMessage<restricted_store> request whendebug({mutils::int_rand()});
				whendebug(logfile << "sending id " <<
									phase::txnID::value << " with nonce " << request.txn_nonce << " for phase " << phase{} << std::endl);
				constexpr typename phase::requirements requires;
				send_store_values(requires);
				s.copy_to(*request.store);
				c.send(phase::txnID::value, request.bytes_size(), request);
				std::size_t size{0};
				c.receive(size);
				auto reply = c.template receive<ServerReplyMessage<Name,restricted_store> >(dsm,size);
				assert(reply->txn_nonce == request.txn_nonce);
				s.update_with(*reply->store);
			}
			
		}}}
