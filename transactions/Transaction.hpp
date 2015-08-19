#pragma once

#include "args-finder.hpp"
#include "../BitSet.hpp"
#include "Transaction_macros.hpp"

struct Transaction{
	const std::function<bool (const BitSet<HandleAbbrev> &, const BitSet<HandleAbbrev> &,Store &)> action;
	const std::function<std::ostream & (std::ostream &os)> print;
	const BitSet<HandleAbbrev> strong;
	const BitSet<HandleAbbrev> weak;
	
	template<typename Cmds>
	Transaction(const TransactionBuilder<Cmds> &s):
		action([s](const BitSet<HandleAbbrev> &, const BitSet<HandleAbbrev> &, Store &st) -> bool{
				//TODO: beginning and leaving transactions needs to happen.
				//Beginning transaction can just happen via calls to
				//handles (i.e. run_ast).
				//leaving transactions seems trickier.
				//Maybe stash things in the store?

				//NOPE: this is exactly what those readSets are for.
				//that will give you exactly the handles you need
				//to begin and end transactions, which you should
				//really do here.

				Store cache;
				//do something transactiony
				call_all_strong(cache,st,s.curr);
				//do something else transactiony
				return call_all_causal(cache,st,s.curr);
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
