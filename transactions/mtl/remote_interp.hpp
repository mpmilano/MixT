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

			template<typename phase, typename FullStore>
			auto remote_interp(mutils::DeserializationManager* dsm, mutils::connection &c, FullStore &s){
				using namespace mutils;
				using namespace server;
				whendebug(auto &logfile = c.get_log_file(););
				using restricted_store
					= typename FullStore::template restrict_to_phase<phase>;
				whendebug(logfile << "sending with store " << type_name<restricted_store>() << std::endl);
				whendebug(logfile << "sending id " <<
									phase::txnID::value << " for phase " <<
									phase{} << std::endl);
				ClientRequestMessage<restricted_store> request;
				s.copy_to(*request.store);
				c.send(phase::txnID::value, request.bytes_size(), request);
				std::size_t size{0};
				c.receive(size);
				auto reply = c.template receive<ServerReplyMessage<Name,restricted_store> >(dsm,size);
				s.update_with(*reply->store);
			}
			
		}}}
