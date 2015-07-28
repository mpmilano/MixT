#pragma once
#include "tuple_extras.hpp"
#include "utils.hpp"
#include "args-finder.hpp"
#include "ConStatement.hpp"
#include "filter-varargs.hpp"

//because we don't have a lot of inspection abilities into this function,
//we only allow write-enabled handles into which *all* read-enabled handles are
//permitted to flow.


//Idea: you can have template<...> above this and it will work!

#define OPERATION(Name, args...) auto Name(args) {	\
	struct r { static bool f(args)

#define END_OPERATION };								\
	auto fp = r::f;										\
	return Operation<decltype(fp)>(fp);					\
	}


//TODO: extract_robj_p actually should worry about perfect forwarding, probably.

template<typename>
struct Operation;

//TODO: perfect forwarding? 
template<typename T, restrict(!is_handle<T>::value)>
 auto extract_robj_p(const T& t) {
		return t;
}

template<typename T, restrict2(!is_handle<T>::value)>
 auto extract_robj_p(T& t)  {
	return t;
}

template<HandleAccess ha, Level l, typename T>
 auto extract_robj_p(const Handle<l,ha,T>& t)  {
	return &t.remote_object();
}

template<typename Ret, typename... A>
struct Operation<Ret (*) (A...)> {
	typedef Ret (*F) (A...);
	static constexpr int arity = sizeof...(A);
	typedef std::tuple<A...> args_tuple;

	std::function<Ret (A...)> fun;
	
	Operation(Ret (*fun) (A...)):fun(fun) {}

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
	auto operator()(Args & ... args) const {
		static_assert(sizeof...(Args) == arity, "Error: arity violation");
		typedef fold_types<type_check,std::tuple<Args...>,
						   std::pair<args_tuple, std::true_type> >
			ft_res;
		static_assert(Right<ft_res>::value,
					  "Error: TypeError calling operation!");
		constexpr Level min = min_level<typename
										filter<is_readable_handle,Args...>::type
										>::value;
		constexpr Level max = max_level<typename
										filter<is_writeable_handle,Args...>::type
										>::value;
		static_assert(can_flow(min,max),"Error: potential flow violation!");
		assert(can_flow(min,max));
		
		return fun(extract_robj_p(args)...);
	}
};
