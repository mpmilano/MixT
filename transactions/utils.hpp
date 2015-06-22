#pragma once
#include <type_traits>

template<typename T, std::size_t size1, std::size_t size2>
std::array<T,size1 + size2> prefix_array(const std::array<T,size1>& t, const std::array<T,size2> &arr){
	std::array<T, size1 + size2> ret;
	std::size_t i = 0;
	for (; i < size1; ++i){
		ret[i] = t[i];
	}
	std::size_t j = 0;
	for (; i < size1 + size2; ++i){
		ret[i] = arr[j];
		++j;
	}
	return ret;
}

#define restrict(x) typename ignore = typename std::enable_if<x>::type
