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

	template<typename T>
	T operator/(const T& t) const {
		return t;
	}

	constexpr bool strongCall(const Store&, const Store&) const {
		return true;
	}
	constexpr bool causalCall(const Store&, const Store&) const {
		return true;
	}

	template<Level l2>
	friend std::ostream & operator<<(std::ostream &os, const Noop<l2>&);
};

template<unsigned long long ID, Level l>
struct contains_temporary<ID, Noop<l> > : std::false_type {};

template<unsigned long long ID, Level l>
auto find_usage(const Noop<l>&){
	return nullptr;
}

const Noop<Level::strong> dummy1;
const Noop<Level::causal> dummy2;


template<typename T>
struct is_Noop : std::integral_constant<
	bool,
	std::is_same<T,Noop<Level::strong> >::value ||
	std::is_same<T,Noop<Level::causal> >::value>::type {};



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

#define CONNECTOR_OP 
