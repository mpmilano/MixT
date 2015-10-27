#pragma once

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


struct Transaction{
	const std::function<bool ()> action;
	const std::function<std::ostream & (std::ostream &os)> print;
	
	template<typename Cmds>
	Transaction(const TransactionBuilder<Cmds> &s):
		action([s]() -> bool{

				debug_forbid_copy = true;
				AtScopeEnd ase{[](){debug_forbid_copy = false;}};
				discard(ase);

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

				{
				bool any = false;

				//TODO: it'd probably be better to keep the association of RemoteObject
				//to TransactionContext here, and to pass the transactionContext
				//in when performing operations inside a transaction
				
				//this loop finds all stores, calls begin_transaction on them exactly once,
				//and sets their participating RemoteObjects' current transaction pointers.

				std::set<std::shared_ptr<GeneralRemoteObject> > collected_objs;
				foreach(s.curr, [&](const auto &e){
						foreach(e.handles(),[&](const auto &h){
								any = true;
								collected_objs.insert(h._ro);
							});});
				
				for (auto &_ro : collected_objs){
					auto *sto = &_ro->store();
					auto *ro = _ro.get();
					old_ctx[ro] = ro->currentTransactionContext();
					if (tc_without(sto)){
						tc[sto->level]
							.emplace(sto, sto->begin_transaction());
					}
					assert(!tc_without(sto));
					assert(ro->currentTransactionContext() == nullptr);
					assert(&ro->store() == sto);
					assert(sto == &tc.at(sto->level).at(sto)->store());
					ro->setTransactionContext(
						tc.at(sto->level).at(sto).get());
				}
				
				assert(any && "no handles traversed");
				}
				
				StrongCache caches;
				StrongStore stores;
				set_context(caches,context::t::unknown);
				call_all_strong(caches,stores,s.curr);
				std::cout << "strong call complete" << std::endl;
				CausalCache &cachec = *((CausalCache*) (&caches));
				CausalStore &storec = *((CausalStore*) (&stores));
				call_all_causal(cachec,storec,s.curr);
				std::cout << "cusal call complete" << std::endl;

				//restore the old transaction pointers
				for (auto &p : old_ctx){
					p.first->setTransactionContext(p.second);
					assert(p.first->currentTransactionContext() == nullptr);
				}


				//todo: REPLICATE THIS COMMIT
				bool ret = true;
				{
					bool any = false;
					if (tc.count(Level::strong) != 0){
						any = true;
						for (auto &p : tc.at(Level::strong)){
							ret = ret && p.second->commit();
						}
					}
					
					//causal commits definitionally can't fail!
					if (ret && tc.count(Level::causal) != 0){
						any = true;
						for (auto &p : tc.at(Level::causal)){
							ret = ret && p.second->commit();
						}
					}
					

					assert(any);
				}
				std::cout << "commit complete" << std::endl;
				assert(ret);
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
		return action();
	}

	struct CannotProceedError {};
	struct ClassCastException{};
};


#include "HandleCaching.hpp"
