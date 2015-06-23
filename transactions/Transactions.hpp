#pragma once
#include "../Backend.hpp"
#include "utils.hpp"
#include <string>
#include <memory>
#include <tuple>

typedef backend::Level Level;

template<Level l>
struct ConStatement {

};

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

template<typename T, Level level, typename StrongNext, typename WeakNext>
class Seq;

template<Level l, typename T>
static auto make_seq(const T &stm){
	auto d1 = std::make_tuple(dummy1);
	auto d2 = std::make_tuple(dummy2);
	return Seq<T,l, decltype(d1),decltype(d2)>(stm,d1,d2);
}


//StrongNext and WeakNext are tuples of operations.
template<typename T, Level level, typename StrongNext, typename WeakNext>
class Seq {
private:
	const T member;
	const StrongNext strong;
	const WeakNext weak;	

public:
	Seq(const T &mem,
		const StrongNext &sn,
		const WeakNext &wn):
		member(mem),
		strong(sn),
		weak(wn){}
	
private:
	
	template<typename T1, typename other1_strong, typename other1_weak,
			 typename T2, typename other2_strong, typename other2_weak, Level l>
	static auto build_seq(const Seq<T1, l, other1_strong, other1_weak> &o1,
						  const Seq<T2, Level::strong, other2_strong, other2_weak>
						  &o2){
			auto sn =
				std::tuple_cat(o1.strong,
							   std::make_tuple(o2.member),
							   o2.strong);
			auto wn =
				std::tuple_cat(o1.weak,
							   o2.weak);
			return Seq<T1, l, decltype(sn), decltype(wn)>(o1.member,sn,wn);
	}

	template<typename T1, typename other1_strong, typename other1_weak,
			 typename T2, typename other2_strong, typename other2_weak, Level l>
	static auto build_seq(const Seq<T1, l, other1_strong, other1_weak> &o1,
						  const Seq<T2, Level::causal, other2_strong, other2_weak>
						  &o2){
			auto sn =
				std::tuple_cat(o1.strong,o2.strong);
			auto wn =
				std::tuple_cat(o1.weak,
							   std::make_tuple(o2.member),
							   o2.weak);
			return Seq<T1, l, decltype(sn), decltype(wn)>(o1.member,sn,wn);
	}
	
public:


	
	template<typename T2, Level l, typename other_strong, typename other_weak>
	auto operator,(const Seq<T2, l, other_strong, other_weak> &s2) const {
		return build_seq(*this,s2);
	}

	template<typename T2>
	auto operator,(const T2 &stm) const{
		return build_seq(*this,make_seq<Level::causal>(stm));
	}

	template<typename T2, Level l, typename StrongNext2, typename WeakNext2>
	friend class Seq;
};



