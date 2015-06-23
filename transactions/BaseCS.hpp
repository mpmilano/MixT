#pragma once
#include "ConStatement.hpp"


template<Level l>
class Noop : public ConStatement<l> {
public:
	Noop(){}
	bool operator==(const Noop &) const {return true;}
	bool operator==(const ConStatement<l>& c) const {
		if (Noop* n = dynamic_cast<Noop>(&c)) return true;
		else return false;
	}
};

const Noop<Level::strong> dummy1;
const Noop<Level::causal> dummy2;


template<Level l, int i>
class CSInt : public ConStatement<l>, std::integral_constant<int,i>::type {
public:
	CSInt(){}
};
