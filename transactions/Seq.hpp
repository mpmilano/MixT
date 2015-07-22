#pragma once
#include "ConStatement.hpp"
#include "BaseCS.hpp"
#include "../BitSet.hpp"
#include "tuple_extras.hpp"


//holes; for filling in variables. 
template<typename StrongNext, typename WeakNext>
struct Seq;


template<typename T,backend::Level l,typename>
Seq<std::tuple<T>, std::tuple<> > make_seq(const T &stm){
	static_assert(l == Level::strong);
	static_assert(is_ConStatement<T>::value,"ugh restrict is broken");
	assert(is_ConStatement<T>::value);
	return Seq<std::tuple<T>, std::tuple<> >(std::tuple<T>(stm), std::make_tuple());
}

template<typename T,backend::Level l,typename>
Seq<std::tuple<>, std::tuple<T> > make_seq(const T &stm){
	static_assert(l == Level::causal);
	static_assert(is_ConStatement<T>::value,"ugh restrict is broken");
	assert(is_ConStatement<T>::value);
	return Seq<std::tuple<>, std::tuple<T> >(std::make_tuple(), std::make_tuple(stm));
}


template<typename StrongNext, typename WeakNext>
auto make_seq(const Seq<StrongNext, WeakNext> &stm) {
	return Seq<StrongNext, WeakNext>(stm);
}

template<typename>
struct is_If;

template<typename other1_strong, typename other1_weak,
		 typename other2_strong, typename other2_weak>
auto seq_cat(const Seq<other1_strong, other1_weak> &o1,
					const Seq<other2_strong, other2_weak> &o2){
	auto sn =
		std::tuple_cat(o1.strong, o2.strong);
	auto wn =
		std::tuple_cat(o1.weak, o2.weak);
	return Seq<decltype(sn), decltype(wn)>(sn,wn);
}


//StrongNext and WeakNext are tuples of operations.
template<typename StrongNext, typename WeakNext>
struct Seq {
	static_assert(is_cs_tuple<StrongNext>::value,"Need to be a CS tuple!");
	static_assert(is_cs_tuple<WeakNext>::value,"Need to be a CS tuple!");

	const StrongNext strong;
	const WeakNext weak;
	
	Seq(const StrongNext &sn, const WeakNext &wn)
		:strong(sn), weak(wn)
		{}
	
	template<typename other_strong, typename other_weak>
	auto operator/(const Seq<other_strong, other_weak> &s2) const {
		auto s = fold(s2.strong,[](const auto &e, const auto &acc){
				return acc / e;
			},*this);
		auto w = fold(s2.weak,[](const auto &e, const auto &acc){
				return acc / e;
			},s);
		return w;
	}

	template<typename T2,
			 restrict(is_ConStatement<T2>::value  && 
					  !is_Noop<T2>::value &&
					  !is_If<T2>::value)>
	auto operator/(const T2 &stm) const{
		assert(is_ConStatement<T2>::value);
		return seq_cat(*this,make_seq(stm));
	}

	template<typename T2,
			 restrict(is_Noop<T2>::value)>
	auto operator/(T2) const{
		return *this;
	}

	template<typename T>
	auto operator/(const ReplaceMe<T> &rm) const{
		return seq_cat (*this, make_seq(replace(*this,rm.t)));
	}

	auto getStrongReadSet() const {
		return fold(strong,
					[](const auto &e, const auto &acc)
					{return acc.addAll(e.getReadSet());},
					BitSet<backend::HandleAbbrev>());
	}
	
	auto getWeakReadSet() const {
		return fold(weak,
					[](const auto &e, const auto &acc)
					{return acc.addAll(e.getReadSet());},
					BitSet<backend::HandleAbbrev>());
	}

	static constexpr auto size() {
		return std::tuple_size<StrongNext>::value +
			std::tuple_size<WeakNext>::value;
	}

	template<typename F, typename Acc>
	auto ifold(const F &f, const Acc &acc) const{
		return fold(weak,f,fold(strong,f,acc));
	}

	bool operator()(Store &st) const {
		//TODO: I assume there's something fancier I need
		//to do here based on backing stores and such,
		//using the accumulated readsets.
		auto fun = [&st](const auto &s, const auto &acc){return s(st) && acc;};
		return fold(strong,fun,true) && fold(weak,fun,true);
	}

/*
	
	template<typename StrongNext2, typename WeakNext2>
	friend class Seq;

	template<typename StrongNext2, typename WeakNext2>
	friend std::ostream & operator<<(std::ostream &os,
									 const Seq<StrongNext2,WeakNext2>& i);
	
	template<typename T, backend::Level, typename ig>
	friend Seq<std::tuple<>, std::tuple<T> > make_seq(const T &);
	template<typename T, backend::Level, typename ig>
	friend Seq<std::tuple<T>, std::tuple<> > make_seq(const T &);
		
	friend const Seq<std::tuple<>,std::tuple<> > & empty_seq();

*/
};


const Seq<std::tuple<>,std::tuple<> > & empty_seq(){
	static Seq<std::tuple<>,std::tuple<> > ret(std::make_tuple(), std::make_tuple());
	return ret;
}

template<typename T>
auto strip_seq(const Seq<std::tuple<T>,std::tuple<> > &s){
	return std::get<0>(s.strong);
}

template<typename T>
auto strip_seq(const Seq<std::tuple<>,std::tuple<T> > &s){
	return std::get<0>(s.weak);
}

template<typename T>
auto strip_seq(const T &s){
	return s;
}


template<typename StrongNext2, typename WeakNext2>
std::ostream & operator<<(std::ostream &os,
						  const Seq<StrongNext2,WeakNext2>& i){

	auto f = [&](const auto &e, const auto& ){
		os << "   " << e << ";" << std::endl;
		return 0;
	};

	os << "Strong: {" << std::endl;
	fold(i.strong,f,0);
	os << "}" << std::endl << "Weak: {" << std::endl;
	fold(i.weak, f, 0);
	return os << "}";
}
