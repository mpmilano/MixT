#pragma once
#include "../Backend.hpp"
#include "utils.hpp"
#include <string>

#define Statement std::string

typedef backend::Level Level;

template<Level l>
struct ConStatement : Statement {
	ConStatement(const Statement& s):Statement(s){}
};
constexpr Level strong = Level::strong;
constexpr Level weak = Level::causal;

const ConStatement<strong> dummy1("dummy");
const ConStatement<weak> dummy2("dummy");

template<std::size_t size_strong, std::size_t size_weak>
class Seq {
private:
	const std::array<ConStatement<strong>, size_strong> strong_statements;
	const std::array<ConStatement<weak>, size_weak> weak_statements;
public:

	template<std::size_t othersize1_strong, std::size_t othersize1_weak,
			 std::size_t othersize2_strong, std::size_t othersize2_weak >
	Seq(const std::array<ConStatement<strong>,othersize1_strong> &stmt_strong,
		const std::array<ConStatement<strong>,othersize2_strong> &stmt2_strong,
		const std::array<ConStatement<weak>,othersize1_weak> &stmt_weak,
		const std::array<ConStatement<weak>,othersize2_weak> &stmt2_weak):
		strong_statements(prefix_array(stmt_strong,stmt2_strong)),
		weak_statements(prefix_array(stmt_weak,stmt2_weak))
		{}

	template<std::size_t othersize_strong, std::size_t othersize_weak>
	Seq<size_strong + othersize_strong, size_weak + othersize_weak>
		operator,(const Seq<othersize_strong, othersize_weak> &s2) const {
		return Seq<size_strong + othersize_strong,size_weak + othersize_weak>
			(strong_statements, s2.strong_statements,
			 weak_statements, s2.weak_statements);
	}

	Seq<size_strong + 1, size_weak> operator,(const ConStatement<strong> &stm) const{
		std::array<ConStatement<strong>,1> tmp {{stm}};
		return Seq<size_strong + 1, size_weak>
			(strong_statements,tmp,weak_statements,{});
	}

	Seq<size_strong, size_weak+ 1> operator,(const ConStatement<weak> &stm) const{
		std::array<ConStatement<weak>,1> tmp {{stm}};
		return Seq<size_strong, size_weak + 1>
			(strong_statements,{},weak_statements, tmp);
	}

	Seq(const ConStatement<strong> &stm):strong_statements{{dummy1,stm}},weak_statements{{dummy2}}{}
	
	Seq(const ConStatement<weak> &stm):strong_statements{{dummy1}},weak_statements{{dummy2,stm}}{}
};
