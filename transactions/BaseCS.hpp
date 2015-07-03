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

	BitSet<backend::HandleAbbrev> getReadSet() const {
		return BitSet<backend::HandleAbbrev>();
	}

	template<Level l2>
	friend std::ostream & operator<<(std::ostream &os, const Noop<l2>&);
};

template<Level l>
std::ostream & operator<<(std::ostream &os, const Noop<l>&){
	
	return os << "Noop@" << (l == Level::strong ? "strong" : "weak");
}

const Noop<Level::strong> dummy1;
const Noop<Level::causal> dummy2;


template<Level l, int i>
class CSInt : public ConStatement<l>, public std::integral_constant<int,i>::type {
public:
	CSInt(){}

	BitSet<backend::HandleAbbrev> getReadSet() const {
		return BitSet<backend::HandleAbbrev>();
	}
	
	template<Level l2, int i2>
	friend std::ostream & operator<<(std::ostream &os, const CSInt<l2,i2>&);
};

template<Level l, int i>
std::ostream & operator<<(std::ostream &os, const CSInt<l,i>&){
	return os << i;
}

template<Level l, int i>
constexpr bool is_base_CS_f(const CSInt<l,i>* ){
	return true;
}

template<Level l>
constexpr bool is_base_CS_f(const Noop<l>* ){
	return true;
}

template<typename T>
constexpr bool is_base_CS_f(const T* ){
	return false;
}

template<typename T>
struct is_base_CS : std::integral_constant<bool, is_base_CS_f((T*)nullptr)> {};
