#pragma once

#include "utils.hpp"
#include "tuple_extras.hpp"

template <template<typename> typename Predicate, typename...> struct filter;

template <template<typename> typename Pred> struct filter<Pred> { using type = std::tuple<>; };

template <template<typename> typename Predicate, typename Head, typename... Tail>
struct filter<Predicate, Head, Tail...>
{
	using type = typename std::conditional<Predicate<Head>::value,
										   typename Cons<Head, typename filter<Predicate,Tail...>::type>::type,
										   typename filter<Predicate,Tail...>::type
										   >::type;
};

