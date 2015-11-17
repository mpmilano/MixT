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

private:
	static void assign_current_context (std::map<GeneralRemoteObject<Level::strong>*, TransactionContext*> &old_ctx_s,
								 const std::map<GeneralRemoteObject<Level::causal>*, TransactionContext*> &,
								 GeneralRemoteObject<Level::strong>* ro){
		old_ctx_s[ro] = ro->currentTransactionContext();
	}
	static void assign_current_context (std::map<GeneralRemoteObject<Level::strong> * , TransactionContext*> &,
								 std::map<GeneralRemoteObject<Level::causal>*, TransactionContext*> &old_ctx_c,
								 GeneralRemoteObject<Level::causal>* ro){
		old_ctx_c[ro] = ro->currentTransactionContext();
	}

	static void collected_objs_insert(std::set<std::shared_ptr<GeneralRemoteObject<Level::strong> > > &collected_objs,
							   const std::set<std::shared_ptr<GeneralRemoteObject<Level::causal> > > &,
							   const std::shared_ptr<GeneralRemoteObject<Level::strong> > &ro){
		collected_objs.insert(ro);
	}

	static void collected_objs_insert(const std::set<std::shared_ptr<GeneralRemoteObject<Level::strong> > > &,
							   std::set<std::shared_ptr<GeneralRemoteObject<Level::causal> > > &collected_objs,
							   const std::shared_ptr<GeneralRemoteObject<Level::causal> > &ro){
		collected_objs.insert(ro);
	}
	
public:
	
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

				std::map<GeneralRemoteObject<Level::strong>*, TransactionContext*> old_ctx_s;
				std::map<GeneralRemoteObject<Level::causal>*, TransactionContext*> old_ctx_c;

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

				std::set<std::shared_ptr<GeneralRemoteObject<Level::strong> > > collected_objs_s;
				std::set<std::shared_ptr<GeneralRemoteObject<Level::causal> > > collected_objs_c;
				foreach(s.curr, [&](const auto &e){
						foreach(e.handles(),[&](const auto &h){
								any = true;
								Transaction::collected_objs_insert(collected_objs_s, collected_objs_c,h._ro);
							});});

				auto collected_objs_proc = [&](auto &_ro){
					auto *sto = &_ro->store();
					auto *ro = _ro.get();
					assign_current_context(old_ctx_s,old_ctx_c,ro);
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
				};
				for (auto &_ro : collected_objs_s){
					collected_objs_proc(_ro);
				}
				for (auto &ro : collected_objs_c){
					collected_objs_proc(ro);
				}
				
				assert(any && "no handles traversed");
				}
				
				StrongCache caches;
				StrongStore stores;
				set_context(caches,context::t::unknown);
				call_all_strong(caches,stores,s.curr);
				CausalCache &cachec = *((CausalCache*) (&caches));
				CausalStore &storec = *((CausalStore*) (&stores));
				call_all_causal(cachec,storec,s.curr);

				//restore the old transaction pointers
				for (auto &p : old_ctx_s){
					p.first->setTransactionContext(p.second);
					assert(p.first->currentTransactionContext() == nullptr);
				}
				for (auto &p : old_ctx_c){
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

	struct CannotProceedError {
		const std::string why;
	};
	struct SerializationFailure {};
	struct ClassCastException{};
};


#include "HandleCaching.hpp"
