#pragma once

#include "Seq.hpp"
#include "args-finder.hpp"
#include "../BitSet.hpp"
#include "Operate.hpp"
#include "Transaction_macros.hpp"

struct Transaction{
	const std::function<bool (Store &)> action;
	const std::function<std::ostream & (std::ostream &os)> print;
	const BitSet<HandleAbbrev> strong;
	const BitSet<HandleAbbrev> weak;
	
	template<typename Cmds, typename Vars>
	Transaction(const TransactionBuilder<Cmds,Vars> &s):
		action([s](Store &st) -> bool{
				fold(s.curr,[&st](const auto &e, bool b)
					 {return b && e(st);},true);
			}),
		print([s](std::ostream &os) -> std::ostream& {
				return fold(s.curr,[&os](const auto &e, int)
							{os << e << std::endl; },0);
			}),
		strong(fold(s.curr),
			   [](const auto &e, const auto &bs)
			   {return (e.level == Level::strong ?
						bs.addAll(e.getReadSet()) : bs);},
			   BitSet<HandleAbbrev>()),
		weak(fold(s.curr),
			   [](const auto &e, const auto &bs)
			   {return (e.level == Level::causal ?
						bs.addAll(e.getReadSet()) : bs);},
			   BitSet<HandleAbbrev>()),
		{}

	Transaction(const Transaction&) = delete;

	bool operator()() const {
		Store s;
		return action(s);
	}

	struct CannotProceedError {};
	struct ClassCastException{};
};


std::ostream & operator<<(std::ostream &os, Transaction& t){
	return t.print(os);
}
