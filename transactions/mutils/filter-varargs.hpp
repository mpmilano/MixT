#pragma once

#include "utils.hpp"
#include "tuple_extras.hpp"

namespace mutils{

	template <template<typename> class Predicate, typename...> struct filter;

	template <template<typename> class Pred> struct filter<Pred> { using type = std::tuple<>; };

	template <template<typename> class Predicate, typename Head, typename... Tail>
	struct filter<Predicate, Head, Tail...>
	{
		using type = typename std::conditional<Predicate<Head>::value,
											   typename Cons<Head, typename filter<Predicate,Tail...>::type>::type,
											   typename filter<Predicate,Tail...>::type
											   >::type;
	};


	template <template<typename> class Predicate, typename Head, typename... Tail>
	struct filter<Predicate, std::tuple<Head, Tail...> > : filter<Predicate,Head,Tail...> {};

	template <template<typename> class Predicate, typename Tail>
	using filter_t = filter<Predicate,Tail>; 
}
