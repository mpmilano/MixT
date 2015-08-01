#pragma once
#include "tuple_extras.hpp"
#include "utils.hpp"
#include "args-finder.hpp"
#include "ConStatement.hpp"
#include "filter-varargs.hpp"
#include "type_utils.hpp"

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

template<typename>
struct Operation;

template<typename T, restrict(!is_handle<decay<T> >::value)>
 auto extract_robj_p( T&& t) {
	return std::forward<T>(t);
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
								  sassert2(___T, First<Left<Acc> >, 
										   (std::is_same<decay<___T>, decay<First<Left<Acc> > > >::value || 
											(is_handle<decay<___T> >::value &&
											 is_RemoteObj_ptr<First<Left<Acc> > >::value))) && 
								  Right<Acc>::value
								  > > ;
	
	template<typename... Args>
	auto operator()(Args && ... args) const {
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


#define DOBODY1													\
	//auto Name(args...)										\
//to-do: limit recursion. Hidden boolean argument or something. \
	auto *first_try =											\
		fold(mke<std::tuple<STORE_LIST> >(),					\
		[&](const auto &arg, const auto *accum){				\
		if (!accum){											\
		try {															\
			typedef decay<decltype(arg)> Store;							\
			return heap_copy(
                //Name(args...);
#define DOBODY2 ) ;														\
		}																\
		catch (ClassCastException e){									\
			return accum;												\
		}}},nullptr);													\
	if (first_try){														\
		std::unique_ptr<decltype(*first_try)> ret(first_try);			\
		return ret;														\
	}																	\
	else {																\
		struct Name ## OperationNotSupported {};						\
		throw Name ## OperationNotSupported();							\
	}																	\
}
				
#define DECLARE_OPERATION2(Name, arg)			\
	auto Name (arg a) { DOBODY1					\
	Name(Store::tryCast(a))						\
		DOBODY2

#define DECLARE_OPERATION3(Name,Arg1, Arg2)	 	\
	auto Name (Arg1 a, Arg2 b) { DOBODY1		\
	Name(Store::tryCast(a),Store::tryCast(b))	\
		DOBODY2				


#define DECLARE_OPERATION_IMPL2(count, ...) DECLARE_OPERATION ## count (__VA_ARGS__)
#define DECLARE_OPERATION_IMPL(count, ...) DECLARE_OPERATION_IMPL2(count, __VA_ARGS__)
#define DECLARE_OPERATION(...) DECLARE_OPERATION_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__)

