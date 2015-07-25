#pragma once
#include "tuple_extras.hpp"
#include "utils.hpp"
#include "args-finder.hpp"
#include "ConStatement.hpp"
#include "filter-varargs.hpp"
#include "compile-time-lambda.hpp"

//because we don't have a lot of inspection abilities into this function,
//we only allow write-enabled handles into which *all* read-enabled handles are
//permitted to flow.


//Idea: you can have template<...> above this and it will work!
#define OPERATION(Name, args...) auto Name(args) { \
	static constexpr auto fun = STATIC_LAMBDA(args)

#define END_OPERATION ;											\
	typedef function_traits<decltype(fun)>	ft;					\
	return Operation<ft>();												\
	}


//TODO: extract_robj_p actually should worry about perfect forwarding, probably.

template<typename ft>
struct Operation {
	template<typename ___T, typename Acc>
	using type_check = std::pair<Rest<Left<Acc> >,
								 std::integral_constant
								 <bool,
								  (std::is_same<___T,First<Left<Acc> > >::value ||
								   (is_handle<___T>::value &&
									is_RemoteObj_ptr<First<Left<Acc> > >::value)) &&
								  Right<Acc>::value
								  > > ;
	template<typename... Args>
	auto operator()(Args... args) const {
		static_assert(sizeof...(Args) == ft::arity, "Error: arity violation");
		typedef fold_types<type_check,std::tuple<Args...>,
						   typename ft::args_tuple>
			ft_res;
		static_assert(Right<ft_res>::value,
					  "Error: TypeError calling operation!");
		static_assert(
			can_flow(min_level<filter<is_readable_handle,Args...> >::value,
					 max_level<filter<is_writeable_handle,Args...> >::value),
			"Error: potential flow violation!");
		return fun(extract_robj_p(args)...);
	}
};
