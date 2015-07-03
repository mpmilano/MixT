#pragma once
#include <bitset>
#include "transactions/utils.hpp"

template<typename T>
struct BitSet {

	typedef __int128_t member_t;
	
	const member_t members = 0;

	BitSet(decltype(members) m):members(m){
		assert(can_bitset<T>());
		assert(T::numbits <= sizeof(member_t));
	}

	BitSet():members(0){}
	
	BitSet insert(const T& t) const {
		return members | t;
	}

	bool contains(const T& t) const {
		return (members & t) != 0;
	}

	BitSet addAll(const BitSet &b) const {
		return members & b.members;
	}

	BitSet remove(const T& t) const{
		return (contains(t) ? members - t : *this);
	}
};

template<typename T1>
typename std::enable_if<can_bitset<T1>(),BitSet<T1> >::type
setify(const T1 &only){
	return BitSet<T1>{only};
}

template<typename T1, typename... T>
typename std::enable_if<can_bitset<T1>(),BitSet<T1> >::type
setify(const T1 &first, const T & ... rest){
	return (BitSet<T1>{first}).addAll(setify(rest...));
}


template<typename T, typename O>
BitSet<T> set_union(const BitSet<T> &a, const O &b){
	return BitSet<T>(a).addAll(b);
}

template<typename T, typename A, typename B>
auto set_union(const BitSet<T> &a, const A &b, const B &c){
	return set_union(a,set_union(b,c));
}
