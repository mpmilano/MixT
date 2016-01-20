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
#include "Context.hpp"
#include "Tracker.hpp"


namespace myria { namespace mtl {


		struct Transaction{
			const std::function<bool ()> action;
			const std::function<std::ostream & (std::ostream &os)> print;
			
		public:
	
			template<typename Cmds>
			Transaction(const TransactionBuilder<Cmds> &s):
				action([s]() -> bool{

						debug_forbid_copy = true;
						mutils::AtScopeEnd ase{[](){debug_forbid_copy = false;}};
						discard(ase);

						//We're assuming that operations behave normally,
						//By which we mean if they need to handle in-a-transaction
						//in a special way, they do that for themselves.

						auto& trk = tracker::Tracker::global_tracker();
						TransactionContext ctx{trk.generateContext()};

						StrongCache caches;
						StrongStore stores;
						{
							//strong execution begins
							
							//TODO: it'd probably be better to keep the association of RemoteObject
							//to TransactionContext here, and to pass the transactionContext
							//in when performing operations inside a transaction

							
							//this loop finds all stores, calls begin_transaction on them exactly once,
							//and sets their participating RemoteObjects' current transaction pointers.
							
							set_context(caches,context::t::unknown);
							//nobody should be in a transaction yet
							assert(trk.get_StrongStore().in_transaction() ?
								   [](){static bool first_transaction = true;
									   bool ret = first_transaction;
									   first_transaction = false;
									   return ret;}() : true);
							assert(!trk.get_StrongStore().in_transaction());
							call_all_strong(&ctx,caches,stores,s.curr);

						}

						do {
							//causal execution.  it can't fail.
							
							CausalCache cachec{caches};
							CausalStore storec{stores};
							//causal should also not be in a transaction yet
							assert(!trk.get_CausalStore().in_transaction());
							call_all_causal(&ctx,cachec,storec,s.curr);
							
							//causal commits definitionally can't fail!
							//so we'll just try the causal transaction again if it does!
							using namespace std::chrono;
							auto backoff = 2us;
							
							try{ 
								ctx.full_commit();
								break;
							}
							catch (const StrongFailureError& e){
								//the strong component has failed, and that's not getting re-executed, so let's bail.
								throw e;
							}
							catch (...){
								std::this_thread::sleep_for(backoff);
								backoff *= 2;
								//we really don't want this to fail guys.
							}
						} while (true);
						
						return true;
					}),
				print([s](std::ostream &os) -> std::ostream& {
						os << "printing AST!" << std::endl;
						mutils::fold(s.curr,[&os](const auto &e, int) -> int
									 {os << e << std::endl; return 0; },0);
						os << "done printing AST!" << std::endl;
						return os;
					})
				{}

			Transaction(const Transaction&) = delete;

			bool operator()() const {
				return action();
			}

			struct StrongFailureError: mutils::StaticMyriaException<MACRO_GET_STR("Error: Commit failure on strong portion") >{};

			struct CannotProceedError : mutils::MyriaException{
				const std::string why;
				CannotProceedError(decltype(why) w):why(w){}
				const char* what() const _NOEXCEPT {
					return why.c_str();
				}
			};
			struct SerializationFailure : mutils::StaticMyriaException<MACRO_GET_STR("Error: Serialization Failure")> {};
			struct ClassCastException : mutils::StaticMyriaException<MACRO_GET_STR("Error: Class Cast Exception")> {};
		};




	} }
