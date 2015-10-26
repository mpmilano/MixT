#pragma once
#include <type_traits>

template<typename T, typename V>
struct TrivialPair{
	T first;
	V second;
};

static_assert(std::is_trivially_copyable<TrivialPair<int,long> >::value,"Error: trivially copyable is more restrictive than we had hoped");

template<typename T, typename U, typename V>
struct TrivialTriple{
	T first;
	U second;
	V third;
};
