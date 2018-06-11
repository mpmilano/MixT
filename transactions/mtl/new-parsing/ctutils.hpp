#pragma once
#include <cstddef>
#include "mutils/type_utils.hpp"
template<typename...> struct typelist_ctutils;

template<typename _hd, typename... args>
struct typelist_ctutils<_hd,args...>{
	constexpr typelist_ctutils(){}
	template<typename... moreargs>
	static constexpr auto concat(const typelist_ctutils<moreargs...>&){
		return typelist_ctutils<_hd,args...,moreargs...>{};
	}
	static constexpr std::size_t size = sizeof...(args) + 1;
	using hd = _hd;
	using tail = typelist_ctutils<args...> ;
};

template<>
struct typelist_ctutils<>{
	constexpr typelist_ctutils(){}
	template<typename... moreargs>
	static constexpr auto concat(const typelist_ctutils<moreargs...>&){
		return typelist_ctutils<moreargs...>{};
	}
	static constexpr std::size_t size = 0;
};

template<typename A, typename T, template<T> class value_converter, std::size_t index = 0>
constexpr auto array_to_typelist(){
	if constexpr (index < A::array_size){
		struct arg{
			const DECT(A::array[index]) &e;
			constexpr arg():e(A::array[index]){}
		};
		return typelist_ctutils<value_converter<arg{}.e> >::concat(array_to_typelist<A, T,value_converter, index+1>());
	}
	else return typelist_ctutils<>{};
}

template<typename list, template<typename> class type_converter, typename ret_arr, std::size_t size, std::size_t index>
constexpr auto typelist_to_array_helper(ret_arr (&arr)[size]){
	if constexpr(list::size > 0){
		arr[index] = type_converter<typename list::hd>::value();
		return typelist_to_array_helper<typename list::tail, type_converter, ret_arr, size, index + 1>(arr);
	}
}

template<typename list, template<typename> class type_converter>
constexpr auto typelist_to_array() {
	using returned_type = DECT(type_converter<typename list::hd>::value());
	struct array_container{
		returned_type array[list::size] = {returned_type{}};
		constexpr array_container () = default ;
		constexpr array_container (array_container&&) = default ;
	};
	array_container ret;
	typelist_to_array_helper<list,type_converter, returned_type, list::size,0>(ret.array);
	return ret;
}

namespace test_array_to_typelist {

struct test_array{
	static constexpr int array[5] = {0,0,17,0,0};
	static constexpr int array_size = 5;
};
constexpr int test_array::array[5];
constexpr int test_array::array_size;
template<int i> using converter = std::integral_constant<int,i>;
template<class T> struct reverse_converter {
	static constexpr auto value(){ return T::value;}
};
using test_array_to_typelist_list = DECT(array_to_typelist<test_array, int, converter>());
static_assert(typelist_to_array<test_array_to_typelist_list, reverse_converter>().array[2] == 17) ;
}