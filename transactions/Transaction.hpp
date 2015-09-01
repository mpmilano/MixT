#pragma once

#include "args-finder.hpp"
#include "../BitSet.hpp"
#include "Transaction_macros.hpp"
#include "Tracker.hpp"
#include "DataStore.hpp"
#include "TransactionBasics.hpp"

struct Transaction{
	const std::function<bool (Store &)> action;
	const std::function<std::ostream & (std::ostream &os)> print;
	
	template<typename Cmds>
	Transaction(const TransactionBuilder<Cmds> &s):
		action([s](Store &st) -> bool{


				//We're assuming that operations behave normally,
				//By which we mean if they need to handle in-a-transaction
				//in a special way, they do that for themselves.

				//note: eiger only support read-only or write-only
				//transactions.  which for the moment means we will
				//mark as read/write only the first time we read/write,
				//and just barf if we wind up mixing.

				//TODO: encode that semantics in the type system.


				std::map<GDataStore*,std::unique_ptr<TransactionContext> > tc;

				std::map<GeneralRemoteObject*, TransactionContext*> old_ctx;

				foreach(s.curr, [&](const auto &e){
						foreach(e.handles(),[&](const auto &h){
								auto *sto = &h._ro->store();
								auto *ro = h._ro.get();
								old_ctx[ro] = ro->currentTransactionContext();
								if (tc.count(sto) == 0){
									tc.emplace(sto, sto->begin_transaction());
								}
								ro->setTransactionContext(tc.at(sto).get());
							});});
				
				Store cache;
				call_all_strong(cache,st,s.curr);
				call_all_causal(cache,st,s.curr);

				//restore the old transaction pointers
				for (auto &p : old_ctx){
					p.first->setTransactionContext(p.second);
				}
				
				//todo: end transaction

				//todo: errors
				return true;
				
			}),
		print([s](std::ostream &os) -> std::ostream& {
				os << "printing AST!" << std::endl;
				fold(s.curr,[&os](const auto &e, int) -> int
					 {os << e << std::endl; return 0; },0);
				os << "done printing AST!" << std::endl;
				return os;
			})
		{}

	Transaction(const Transaction&) = delete;

	bool operator()() const {
		Store s;
		return action(s);
	}

	struct CannotProceedError {};
	struct ClassCastException{};
};


