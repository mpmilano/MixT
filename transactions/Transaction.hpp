#pragma once

#include "args-finder.hpp"
#include "../BitSet.hpp"
#include "Transaction_macros.hpp"
#include "Tracker.hpp"
#include "DataStore.hpp"

struct Transaction{
	const std::function<bool (Store &)> action;
	const std::function<std::ostream & (std::ostream &os)> print;
	
	template<typename Cmds>
	Transaction(const TransactionBuilder<Cmds> &s):
		action([s](Store &st) -> bool{

				//We're just having the handles enter transaction when
				//they're encountered in the AST crawl.
				//We're also assuming that operations behave normally
				//while we're doing this.

				//all of which means all we need to do here is coordinate
				//the commit.


				//not quite; I think it would make implementing stores easier if we had all the handles up here.

				auto handles = fold(s.curr,[](const auto &e, const auto &acc){
						return std::tuple_cat(e.handles(),acc);
					},std::tuple<>());
				ignore(handles);
				
				Store cache;
				call_all_strong(cache,st,s.curr);
				call_all_causal(cache,st,s.curr);

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


