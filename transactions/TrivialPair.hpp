#pragma once
#include <type_traits>

template<typename T, typename V>
struct TrivialPair{
	T first;
	V second;
	bool operator==(const TrivialPair& p) const {
		return first == p.first && second == p.second;
	}
	bool operator<(const TrivialPair&p) const {
		return (first < p.first ? true
				: second < p.second);
	}
	bool operator>(const TrivialPair&p) const {
		return p < *this;
	}
};

static_assert(std::is_trivially_copyable<TrivialPair<int,long> >::value,"Error: trivially copyable is more restrictive than we had hoped");

template<typename T, typename U, typename V>
struct TrivialTriple{
	T first;
	U second;
	V third;
	bool operator==(const TrivialTriple& p) const {
		return first == p.first &&
			second == p.second
			&& third == p.third;
	}
	bool operator<(const TrivialTriple&p) const {
		return (first < p.first ? true
				: (second < p.second ? true
				   : third < p.third));
	}
	bool operator>(const TrivialTriple&p) const {
		return p < *this;
	}
	TrivialTriple(T t,U u,V v):first(t),second(u),third(v){}
};
