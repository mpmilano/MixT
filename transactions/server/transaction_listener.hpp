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

namespace myria {

	namespace mtl {namespace runnable_transaction {
			
			template<typename phase1, typename Ctx, typename S>
			void run_phase1(Ctx &ctx, S &s, std::enable_if_t<std::is_void<DECT(run_phase<phase1>(ctx, s))>::value >* = nullptr){
				run_phase<phase1>(ctx, s);
				if (ctx.s_ctx) ctx.s_ctx->store_commit();
			}
			
			template<typename phase1, typename Ctx, typename S>
			auto run_phase1(Ctx &ctx, S &s, std::enable_if_t<!std::is_void<DECT(run_phase<phase1>(ctx, s))>::value >* = nullptr){
				auto ret = run_phase<phase1>(ctx, s);
				if (ctx.s_ctx) ctx.s_ctx->store_commit();
				return ret;
			}
			
			template<typename phase1, typename store>
			auto common_interp(store& s){
				using label = typename phase1::label;
				PhaseContext<label> ctx{};
				do {
					try {
						ctx.reset();
						s.begin_phase();
						return run_phase1<phase1>(ctx,s);
					}
					catch(const SerializationFailure& sf){
						s.rollback_phase();
						whendebug(if (!label::can_abort::value) std::cout << "Error: label which cannot abort aborted! " << label{});
						if (label::can_abort::value) throw sf;
					}
				}
				while(!label::can_abort::value);
			}

		}}
	
	namespace server{

		template<txnID_t txnID, typename phase, typename store>
		struct transaction_listener {

			using label = typename phase::label;
			
			static bool run_if_match(txnID_t id, DeserializationManager& dsm, mutils::connection &c, 
															 char const * const _data){
				if (id == txnID){
					std::size_t request_size = ((std::size_t*)_data)[0];
					auto* data = _data + sizeof(request_size);
					context_ptr<ClientRequestMessage<store> > msg =
						ClientRequestMessage<store>::from_bytes(dsm,data);
					ServerReplyMessage<Name,store> srm{{},std::move(msg->store)};
					
					mtl::runnable_transaction::common_interp<phase, store>(*srm.store);
					/*
					const bool b = _store_diff->complies(*srm.store);
					if(!b){
						assert(false && "populate the cache somehow! Do stuff!");
						}*/
					c.send(srm);
					return true;
				}
				else return false;
			}
		};

		template<typename transaction, typename phase>
		using listener_for = transaction_listener<
			phase::txnID,
			phase,
			typename transaction::all_store::template restrict_to<
				DECT(phase::requirements::combine(phase::owned::combine(typename phase::provides{})))
				>
			>;
	}}
