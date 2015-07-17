#pragma once

#include "Seq.hpp"
#include "args-finder.hpp"
#include "../Backend.hpp"
#include "../BitSet.hpp"

struct Transaction{
	const std::function<bool (Store &)> action;
	const std::function<std::ostream & (std::ostream &os)> print;
	const BitSet<backend::HandleAbbrev> strong;
	const BitSet<backend::HandleAbbrev> weak;

	template<typename S, typename W>
	Transaction(const Seq<S,W> &s):
		action(convert(s)),
		print([s](std::ostream &os) -> std::ostream& {return os << s;}),
		strong(s.getStrongReadSet()),
		weak(s.getWeakReadSet()) {}

	Transaction(const Transaction&) = delete;

	bool operator()() const {
		Store s;
		return action(s);
	}
};

std::ostream & operator<<(std::ostream &os, Transaction& t){
	return t.print(os);
}

#define BEGIN_TRANSACTION { Transaction ____transaction(Noop<backend::Level::strong>() / 
#define END_TRANSACTION Noop<backend::Level::strong>()); std::cout << ____transaction << std::endl;}
