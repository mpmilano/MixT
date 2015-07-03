#pragma once
#include "ConStatement.hpp"
#include "BaseCS.hpp"
#include "../BitSet.hpp"


//holes; for filling in variables. 
template<typename StrongNext, typename WeakNext>
class Seq;


template<typename T,
		 restrict(is_ConStatement<T>::value && get_level<T>::value == Level::strong)>
Seq<std::tuple<T>, std::tuple<> > make_seq(const T &stm){
	static_assert(is_ConStatement<T>::value,"ugh restrict is broken");
	assert(is_ConStatement<T>::value);
	return Seq<std::tuple<T>, std::tuple<> >(std::tuple<T>(stm), std::make_tuple());
}

template<typename T,
		 restrict(is_ConStatement<T>::value && get_level<T>::value == Level::causal)>
Seq<std::tuple<>, std::tuple<T> > make_seq(const T &stm){
	static_assert(is_ConStatement<T>::value,"ugh restrict is broken");
	assert(is_ConStatement<T>::value);
	return Seq<std::tuple<>, std::tuple<T> >(std::make_tuple(), std::make_tuple(stm));
}


template<typename StrongNext, typename WeakNext>
auto make_seq(const Seq<StrongNext, WeakNext> &stm) {
	return Seq<StrongNext, WeakNext>(stm);
}


//StrongNext and WeakNext are tuples of operations.
template<typename StrongNext, typename WeakNext>
class Seq {
	static_assert(is_cs_tuple<StrongNext>::value,"Need to be a CS tuple!");
	static_assert(is_cs_tuple<WeakNext>::value,"Need to be a CS tuple!");
	
public:
	const StrongNext strong;
	const WeakNext weak;
	
private:
	
	BitSet<backend::HandleAbbrev> strongReadSet;
	decltype(strongReadSet) weakReadSet;

	const static auto& accum(){
		static auto ret = [](const auto& e, const decltype(strongReadSet) &acc)
			-> decltype(strongReadSet)
			{
				return set_union(acc,e.getReadSet());
			};
		return ret;
	}
	
	Seq(const StrongNext &sn, const WeakNext &wn)
		:strong(sn), weak(wn),
		 strongReadSet(fold(strong,accum(),decltype(strongReadSet)())),
		 weakReadSet(fold(weak,accum(),decltype(weakReadSet)()))
		{}
	

	
	template<typename other1_strong, typename other1_weak,
			 typename other2_strong, typename other2_weak>
	static auto build_seq(const Seq<other1_strong, other1_weak> &o1,
						  const Seq<other2_strong, other2_weak> &o2){
			auto sn =
				std::tuple_cat(o1.strong, o2.strong);
			auto wn =
				std::tuple_cat(o1.weak, o2.weak);
			return Seq<decltype(sn), decltype(wn)>(sn,wn);
	}
	
public:
	
	template<typename other_strong, typename other_weak>
	auto operator,(const Seq<other_strong, other_weak> &s2) const {
		return build_seq(*this,s2);
	}

	template<typename T2,
			 restrict(is_ConStatement<T2>::value)>
	auto operator,(const T2 &stm) const{
		assert(is_ConStatement<T2>::value);
		return build_seq(*this,make_seq(stm));
	}

	template<typename StrongNext2, typename WeakNext2>
	friend class Seq;

	template<typename StrongNext2, typename WeakNext2>
	friend std::ostream & operator<<(std::ostream &os,
									 const Seq<StrongNext2,WeakNext2>& i);
	
	template<typename T, typename ig>
	friend Seq<std::tuple<>, std::tuple<T> > make_seq(const T &);
	template<typename T, typename ig>
	friend Seq<std::tuple<T>, std::tuple<> > make_seq(const T &);
		
	friend const Seq<std::tuple<>,std::tuple<> > & empty_seq();
};


const Seq<std::tuple<>,std::tuple<> > & empty_seq(){
	static Seq<std::tuple<>,std::tuple<> > ret(std::make_tuple(), std::make_tuple());
	return ret;
}


template<typename StrongNext2, typename WeakNext2>
std::ostream & operator<<(std::ostream &os,
						  const Seq<StrongNext2,WeakNext2>& i){
	return os << "Strong: " << i.strong << "; WEAK: " << i.weak <<";";
}
