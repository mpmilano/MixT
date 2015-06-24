#pragma once
#include <type_traits>
#include <cassert>
#include <tuple>

template<typename T, std::size_t size1, std::size_t size2>
auto prefix_array(const std::array<T,size1>& t,
				  const std::array<T,size2> &arr,
				  const T& remove){
	assert(remove == t[0]);
	assert(remove == arr[0]);
	std::array<T, size1 + size2 - 1 > ret;
	std::size_t i = 0;
	for (; i < size1; ++i){
		ret[i] = t[i];
	}
	std::size_t j = 1;
	for (; i < size1 + size2 -1 ; ++i){
		ret[i] = arr[j];
		++j;
	}
	return ret;
}

namespace aux{
	template<std::size_t...> struct seq{};

	template<std::size_t N, std::size_t... Is>
	struct gen_seq : gen_seq<N-1, N-1, Is...>{};

	template<std::size_t... Is>
	struct gen_seq<0, Is...> : seq<Is...>{};

	template<class Ch, class Tr, class Tuple, std::size_t... Is>
	void print_tuple(std::basic_ostream<Ch,Tr>& os, Tuple const& t, seq<Is...>){
		using swallow = int[];
		(void)swallow{0, (void(os << (Is == 0? "" : ", ") << std::get<Is>(t)), 0)...};
	}
} // aux::

template<class Ch, class Tr, class... Args>
auto operator<<(std::basic_ostream<Ch, Tr>& os, std::tuple<Args...> const& t)
	-> std::basic_ostream<Ch, Tr>&
{
	os << "(";
	aux::print_tuple(os, t, aux::gen_seq<sizeof...(Args)>());
	return os << ")";
}


#define restrict(x) typename ignore = typename std::enable_if<x>::type


template<typename... T>
constexpr bool is_tuple_f(const std::tuple<T...>*){
	return true;
}

template<typename T>
constexpr bool is_tuple_f(const T*){
	return false;
}

template<typename T>
struct is_tuple : std::integral_constant<bool, is_tuple_f((T*) nullptr)>::type {};
//*/

template<int index, int max, typename Acc, typename F, typename Tuple1, typename Tuple2,
		 restrict(index < max)>
auto tuple_2fold_impl(const F& fun, const Tuple1 &t1, const Tuple2 &t2, const Acc &acc, const std::true_type::type* ){
	auto acc2 = fun(std::get<index>(t1),std::get<index>(t2),acc);
	return tuple_2fold_impl<index+1, max, decltype(acc2), F, Tuple1, Tuple2>
		(fun,t1,t2,acc2, (typename std::integral_constant<bool,(index + 1) < max>::type*) nullptr);
}
	
template<int index, int max, typename Acc, typename F, typename Tuple1, typename Tuple2,
			 restrict(index == max)>
auto tuple_2fold_impl(const F& fun, const Tuple1 &t1, const Tuple2 &t2, const Acc &acc, const std::false_type::type*){
	return acc;
}


template<typename Tuple1, typename Tuple2, typename F, typename Acc>
auto tuple_2fold(const F& f, const Tuple1 &t1, const Tuple2 &t2, const Acc &acc){
	static_assert(is_tuple<Tuple1>::value && is_tuple<Tuple2>::value, "These are not tuples.  Try again");
	static_assert(std::tuple_size<Tuple1>::value == std::tuple_size<Tuple2>::value,"Please only double-fold equal-length things");

	constexpr int size = std::tuple_size<Tuple1>::value;
	
	return tuple_2fold_impl<0,size,Acc,F,Tuple1,Tuple2>
		(f,t1,t2,acc,
		 (typename std::integral_constant<bool,(size > 0)>::type*) nullptr);
}
