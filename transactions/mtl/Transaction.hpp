#pragma once

#include <thread>
#include "args-finder.hpp"
#include "../BitSet.hpp"
#include "Transaction_macros.hpp"
#include "DataStore.hpp"
#include "TransactionBasics.hpp"
#include "TransactionBuilder.hpp"
#include "TempBuilder.hpp"
#include "FreeExpr.hpp"
#include "Assignment.hpp"
#include "Tracker.hpp"
#include "EnvironmentExpression.hpp"

namespace myria { namespace mtl {

		/**
		 * For transactions which do not use MTL's information flow.
		 */
		template<typename Strong, typename Causal>
		auto start_transaction(std::unique_ptr<VMObjectLog> log, tracker::Tracker &trk, Strong &strong, Causal &causal){
			using namespace std;
			assert(log);
			auto ctx = make_unique<TransactionContext>(nullptr,trk.generateContext(std::move(log)));
			ctx->strongContext = strong.begin_transaction(ctx->trackingContext->logger,"one-off vm_main transaction");
			ctx->causalContext = causal.begin_transaction(ctx->trackingContext->logger,"one-off vm_main transaction");
			assert(ctx->trackingContext);
			assert(ctx->trackingContext->logger);
			return ctx;
		}

		struct StrongFailureError: mutils::StaticMyriaException<MACRO_GET_STR("Error: Commit failure on strong portion") >{};
		
		struct CannotProceedError : mutils::MyriaException{
			const std::string why;
			CannotProceedError(decltype(why) w):why(w){}
			const char* what() const _NOEXCEPT {
				return why.c_str();
			}
		};

		/**
		 * A full MTL transaction, built using the various builder classes in mtl
		 */
		template<typename T>
		struct Transaction{
			const std::function<
				std::unique_ptr<VMObjectLog>
				(std::unique_ptr<VMObjectLog>, tracker::Tracker&, T const *const)> action;
			const std::function<std::ostream & (std::ostream &os)> print;
			
		public:
	
			template<typename Cmds>
			Transaction(const TransactionBuilder<Cmds> &s):
				action([s, env_exprs = mtl::environment_expressions(s.curr)]
					   (std::unique_ptr<VMObjectLog> log,
						tracker::Tracker& trk, T const * const param) {
						

						//We're assuming that operations behave normally,
						//By which we mean if they need to handle in-a-transaction
						//in a special way, they do that for themselves.

						   log->addField(LogFields::transaction_action,true);
						   TransactionContext ctx{param,trk.generateContext(std::move(log))};

						   static_assert(
							   std::is_same<
							   std::decay_t<decltype(std::get<0>(env_exprs))>,
							   EnvironmentExpression<T> >::value,
							   "Type error on Transaction parameter!");

						StrongCache caches;
						StrongStore stores;
						{
							//strong execution begins
							
							//TODO: it'd probably be better to keep the association of RemoteObject
							//to TransactionContext here, and to pass the transactionContext
							//in when performing operations inside a transaction

							
							//this loop finds all stores, calls begin_transaction on them exactly once,
							//and sets their participating RemoteObjects' current transaction pointers.							

							//nobody should be in a transaction yet
							using namespace mutils;
							
							assert(!trk.get_StrongStore().in_transaction());
							call_all_strong(&ctx,caches,stores,s.curr);

						}

						int causal_count = 0;
						do {

							++causal_count;
							using namespace mutils;

							//causal execution.  it can't fail.
							
							CausalCache cachec{caches};
							CausalStore storec{stores};
							//causal should also not be in a transaction yet
							if (trk.get_CausalStore().in_transaction()){
								std::cerr << "In a transaction when we shouldn't be: " << trk.get_CausalStore().why_in_transaction() << std::endl;
								std::cerr << "My threadID is: " << std::this_thread::get_id() << std::endl;
							}
							assert(!trk.get_CausalStore().in_transaction());
							call_all_causal(&ctx,cachec,storec,s.curr);
							
							//causal commits definitionally can't fail!
							//so we'll just try the causal transaction again if it does!
							using namespace std::chrono;
							auto backoff = 2us;
							
                                                        try {
								ctx.full_commit();
								break;
							}
							catch (const StrongFailureError& e){
								//the strong component has failed, and that's not getting re-executed, so let's bail.
								throw e;
							}
                                                        catch (...) {
								std::this_thread::sleep_for(backoff);
								backoff *= 2;
								//we really don't want this to fail guys.
							}
						} while (true);

                                                auto logger = std::move(ctx.trackingContext->logger);
						logger->addField(LogFields::num_causal_tries,causal_count);
						
						return std::move(logger);
					}),
				print([s](std::ostream &os) -> std::ostream& {
						os << "printing AST!" << std::endl;
						mutils::fold(s.curr,[&os](const auto &e, int) -> int
									 {os << e << std::endl; return 0; },0);
						os << "done printing AST!" << std::endl;
						return os;
					})
				{
				}

			Transaction(const Transaction&) = delete;
			Transaction(const Transaction&& t):action(std::move(t.action)),print(std::move(t.print)){}
			
			auto operator()(std::unique_ptr<VMObjectLog> &&l, tracker::Tracker& trk, T const * const t) const {
				return action(std::move(l),trk,t);
			}
			
		};
		
		struct SerializationFailure : mutils::StaticMyriaException<MACRO_GET_STR("Error: Serialization Failure")> {};
                //struct ClassCastException : mutils::StaticMyriaException<MACRO_GET_STR("Error: Class Cast Exception")> {};

	} }
