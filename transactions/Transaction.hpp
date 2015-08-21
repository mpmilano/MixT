#pragma once

#include "args-finder.hpp"
#include "../BitSet.hpp"
#include "Transaction_macros.hpp"
#include "Tracker.hpp"
#include "DataStore.hpp"

//implementing the tracker bits for transactions here!  Woo code!

void Tracker::markInTransaction(DataStore<Level::strong>& ds) {
	if (!ds.in_transaction()) ds.begin_transaction();
	strongTransStore = &ds;
	//TODO - impl
}

void Tracker::markInTransaction(DataStore<Level::causal>& ds) {
	if (!ds.in_transaction()) ds.begin_transaction();
	causalTransStore = &ds;
	//TODO - impl
}

DataStore<Level::strong>* Tracker::strongStoreInTransaction() {
	//TODO - impl
	return strongTransStore;
}

DataStore<Level::causal>* Tracker::causalStoreInTransaction() {
	//TODO - impl
	return causalTransStore;
}


struct Transaction{
	const std::function<bool (const BitSet<HandleAbbrev> &, const BitSet<HandleAbbrev> &,Store &)> action;
	const std::function<std::ostream & (std::ostream &os)> print;
	const BitSet<HandleAbbrev> strong;
	const BitSet<HandleAbbrev> weak;
	
	template<typename Cmds>
	Transaction(const TransactionBuilder<Cmds> &s):
		action([s](const BitSet<HandleAbbrev> &, const BitSet<HandleAbbrev> &, Store &st) -> bool{

				//We're just having the handles enter transaction when
				//they're encountered in the AST crawl.
				//We're also assuming that operations behave normally
				//while we're doing this.

				//all of which means all we need to do here is coordinate
				//the commit.
				
				Store cache;
				call_all_strong(cache,st,s.curr);
				call_all_causal(cache,st,s.curr);

				//I hope we remembered to put it in there!
				Tracker &t = *cache.get<Tracker*>(-1);
				DataStore<Level::strong> *ss = t.strongStoreInTransaction();
				DataStore<Level::causal> *cs = t.causalStoreInTransaction();
				assert(ss);
				assert(cs);

				//TODO: fault-tolerance
				ss->end_transaction();
				cs->end_transaction();

				//TODO: error propogation
				return true;
				
			}),
		print([s](std::ostream &os) -> std::ostream& {
				os << "printing AST!" << std::endl;
				fold(s.curr,[&os](const auto &e, int) -> int
					 {os << e << std::endl; return 0; },0);
				os << "done printing AST!" << std::endl;
				return os;
			}),
		strong(fold(s.curr,
					[](const auto &e, const auto &bs)
					{return (e.level == Level::strong ?
							 bs.addAll(e.getReadSet()) : bs);},
					BitSet<HandleAbbrev>())),
		weak(fold(s.curr,
				  [](const auto &e, const auto &bs)
				  {return (e.level == Level::causal ?
						   bs.addAll(e.getReadSet()) : bs);},
				  BitSet<HandleAbbrev>()))
		{}

	Transaction(const Transaction&) = delete;

	bool operator()() const {
		Store s;
		return action(strong,weak,s);
	}

	struct CannotProceedError {};
	struct ClassCastException{};
};


std::ostream & operator<<(std::ostream &os, Transaction& t){
	return t.print(os);
}
