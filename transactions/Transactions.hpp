#pragma once
#include "../Backend.hpp"
#include "utils.hpp"
#include <string>
#include <memory>

typedef backend::Level Level;

template<Level l>
class ConStatement {
public:
	virtual bool operator==(const ConStatement&) const = 0;
};

template<Level l>
class Noop : public ConStatement<l>, std::true_type{
public:
	bool operator==(const Noop &) const {return true;}
	bool operator==(const ConStatement<l>& c) const {
		if (Noop* n = dynamic_cast<Noop>(&c)) return true;
		else return false;
	}
};

constexpr Level strong = Level::strong;
constexpr Level weak = Level::causal;

const ConStatement<strong> dummy1 = Noop<strong>();
const ConStatement<weak> dummy2 = Noop<weak>();

template<std::size_t size_strong, std::size_t size_weak>
class Seq {
private:
	const std::array<ConStatement<strong>, size_strong + 1> strong_statements;
	const std::array<ConStatement<weak>, size_weak + 1> weak_statements;
public:

	template<std::size_t othersize1_strong, std::size_t othersize1_weak,
			 std::size_t othersize2_strong, std::size_t othersize2_weak >
	Seq(const Seq<othersize1_strong, othersize1_weak> &a,
		const Seq<othersize2_strong, othersize2_weak> &b):
		strong_statements(
			prefix_array(a.strong_statements,b.strong_statements,dummy1)),
		weak_statements(
			prefix_array(a.weak_statements, b.weak_statements,dummy2))
		{}
	
	template<std::size_t othersize_strong, std::size_t othersize_weak>
	auto operator,(const Seq<othersize_strong, othersize_weak> &s2) const {
		return Seq<size_strong + othersize_strong,size_weak + othersize_weak>
			(*this,s2);
	}
	
	auto operator,(const ConStatement<strong> &stm) const{
		Seq<1,0> tmp(stm);
		return Seq<size_strong + 1, size_weak>(*this,tmp);
	}

	auto operator,(const ConStatement<weak> &stm) const{
		Seq<0,1> tmp(stm);
		return Seq<size_strong, size_weak + 1>(*this,tmp);
	}

	Seq(const ConStatement<strong> &stm):strong_statements({{dummy1,stm}}),weak_statements{{dummy2}}{}
	
	Seq(const ConStatement<weak> &stm):strong_statements{{dummy1}},weak_statements{{dummy2,stm}}{}

	template<std::size_t ss, std::size_t sw>
	friend class Seq;
};

