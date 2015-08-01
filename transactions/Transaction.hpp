#pragma once

#include "Seq.hpp"
#include "args-finder.hpp"
#include "../BitSet.hpp"
#include "Operate.hpp"

struct Transaction{
	const std::function<bool (Store &)> action;
	const std::function<std::ostream & (std::ostream &os)> print;
	const BitSet<HandleAbbrev> strong;
	const BitSet<HandleAbbrev> weak;

	template<typename Arg, typename Accum>
	using Verify =
		typename std::integral_constant
		<bool,(verify_compilation_complete(mke_p<Arg>())) &&
		 Accum::value >::type;
	
	template<typename S, typename W>
	Transaction(const Seq<S,W> &s):
		action(convert(s)),
		print([s](std::ostream &os) -> std::ostream& {return os << s;}),
		strong(s.getStrongReadSet()),
		weak(s.getWeakReadSet())
		{
			static_assert(
				fold_types<Verify,S,std::true_type>::value,
				"Error: semantic validation failed. Likely due to an invalid reference.  Check your variables!"
				);
		}

	Transaction(const Transaction&) = delete;

	bool operator()() const {
		Store s;
		return action(s);
	}

	struct CannotProceedError {};
};



std::ostream & operator<<(std::ostream &os, Transaction& t){
	return t.print(os);
}

#define BEGIN_TRANSACTION { Transaction ____transaction(Noop<Level::strong>() / 
#define END_TRANSACTION Noop<Level::strong>()); ____transaction(); std::cout << ____transaction << std::endl;}
