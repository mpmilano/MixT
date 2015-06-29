#pragma once
#include "ConStatement.hpp"
#include "BaseCS.hpp"


//holes; for filling in variables. 
template<typename StrongNext, typename WeakNext, typename... ReadSet>
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
template<typename StrongNext, typename WeakNext, typename... ReadSet>
class Seq {
	static_assert(is_cs_tuple<StrongNext>::value,"Need to be a CS tuple!");
	static_assert(is_cs_tuple<WeakNext>::value,"Need to be a CS tuple!");
	
public:
	const StrongNext strong;
	const WeakNext weak;
	const std::tuple<ReadSet...> readSet;
	
private:

#define grs(x)								   \
	auto get_readset(const ConStatement<x> &){ \
		return 0;							   \
	}										   \

	grs(Level::strong)

	grs(Level::causal)

	
	
	template<int curr, int size, restrict(curr < size), typename... Members>
	auto get_readset(const std::tuple<Members...> &m, std::true_type*){
		typename std::integral_constant<bool,(curr + 1 < size)>::type* dummy = nullptr;
		return std::tuple_cat(std::make_tuple(get_readset(std::get<curr>(m))),get_readset<curr+1,size>(m,dummy));
	}
		
	template<int curr, int size, restrict2(curr == size), typename... Members>
	auto get_readset(const std::tuple<Members...> &, std::false_type*){
		return std::make_tuple();
	}
	
	auto get_readset(const StrongNext &sn){
		typename std::integral_constant<bool,(0 < std::tuple_size<StrongNext>::value)>::type* dummy = nullptr;
		return get_readset<0,std::tuple_size<StrongNext>::value>(sn,dummy);
	}
	
	Seq(const StrongNext &sn,
		const WeakNext &wn):
		strong(sn),
		weak(wn),
		readSet(std::tuple_cat(get_readset(sn)))
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

	template<typename StrongNext2, typename WeakNext2, typename... ReadSet2>
	friend class Seq;

	template<typename StrongNext2, typename WeakNext2, typename... ReadSet2>
	friend std::ostream & operator<<(std::ostream &os,
									 const Seq<StrongNext2,WeakNext2,ReadSet2...>& i);
	
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


template<typename StrongNext2, typename WeakNext2, typename... ReadSet2>
std::ostream & operator<<(std::ostream &os,
						  const Seq<StrongNext2,WeakNext2,ReadSet2...>& i){
	return os << "Strong: " << i.strong << "; WEAK: " << i.weak <<";";
}
