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

	BitSet<HandleAbbrev> getReadSet() const {
		return BitSet<HandleAbbrev>();
	}

	constexpr bool operator()(Store&) const {
		return true;
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

template<typename T>
struct is_Noop : std::integral_constant<
	bool,
	std::is_same<T,Noop<Level::strong> >::value ||
	std::is_same<T,Noop<Level::causal> >::value>::type {};
							
template<typename StrongNext, typename WeakNext>
struct Seq;

template<typename T,Level l = get_level<T>::value,
		 restrict(is_ConStatement<T>::value && l == Level::causal)>
Seq<std::tuple<>, std::tuple<T> > make_seq(const T &);

template<typename T,Level l = get_level<T>::value,
		 restrict(is_ConStatement<T>::value && l == Level::strong)>
Seq<std::tuple<T>, std::tuple<> > make_seq(const T &);

#define CONNECTOR_OP	template<typename T2>						 \
	auto operator/(const T2 &t) const {								 \
		return (make_seq(*this)) / t;								 \
	}																 \
	auto operator/(const decltype(dummy1)&) const {					 \
		return *this;												 \
	}																 \
	auto operator/(const decltype(dummy2)&) const {					 \
		return *this;												 \
	}																 \
	template<typename T2>											 \
	auto operator,(const T2& t) const {								 \
		return operator/(t);										 \
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
