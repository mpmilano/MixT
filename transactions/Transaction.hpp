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


				std::map<Level,
						 std::map< GDataStore*,
								   std::unique_ptr<TransactionContext> > > tc;

				std::map<GeneralRemoteObject*, TransactionContext*> old_ctx;

				auto tc_without = [&](auto* sto){
					return
					(tc.count(sto->level) == 0) ||
					tc.at(sto->level).count(sto) == 0;
				};

				foreach(s.curr, [&](const auto &e){
						foreach(e.handles(),[&](const auto &h){
								auto *sto = &h._ro->store();
								auto *ro = h._ro.get();
								old_ctx[ro] = ro->currentTransactionContext();
								if (tc_without(sto) == 0){
									tc[sto->level]
										.emplace(sto, sto->begin_transaction());
								}
								ro->setTransactionContext(
									tc.at(sto->level).at(sto).get());
							});});
				
				Store cache;
				call_all_strong(cache,st,s.curr);
				call_all_causal(cache,st,s.curr);

				//restore the old transaction pointers
				for (auto &p : old_ctx){
					p.first->setTransactionContext(p.second);
				}

				//todo: REPLICATE THIS COMMIT
				bool ret = true;
				for (auto &p : tc.at(Level::strong)){
					ret = ret && p.second->commit();
				}

				//causal commits definitionally can't fail!
				if (ret){
					for (auto &p : tc.at(Level::causal)){
					ret = ret && p.second->commit();
					}
				}

				//TODO: exception here instead of boolean?
				return ret;
				
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


