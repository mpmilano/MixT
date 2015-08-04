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

#define OPERATION(Name, args...) auto Name ## _impl (args) { \
	struct r { static bool f(args)

#define END_OPERATION };								\
	auto fp = r::f;										\
	return Operation<extract_store<decltype(fp)>,decltype(fp)>(fp);	\
	}

template<typename, typename>
struct Operation;

template<typename T, restrict(!is_handle<decay<T> >::value)>
 auto extract_robj_p( T&& t) {
	return std::forward<T>(t);
}

template<HandleAccess ha, Level l, typename T>
 auto extract_robj_p(const Handle<l,ha,T>& t)  {
	return &t.remote_object();
}

template<typename>
struct extract_store_str;

template<typename Ret, typename... Args>
struct extract_store_str<Ret (*) (Args...)> {
	typedef extract_match<is_RemoteObj_ptr,Args...> match;
	using type = typename std::remove_pointer<decay<match> >::type::Store;
};

template<typename T>
using extract_store = typename extract_store_str<T>::type;

template<typename Store, typename Ret, typename... A>
struct Operation<Store, Ret (*) (A...)> {
	const bool built_well = false;
	typedef Ret (*F) (A...);
	typedef Ret result_t;
	static constexpr int arity = sizeof...(A);
	typedef std::tuple<A...> args_tuple;
	
	Ret (*fun) (A...);
	
	Operation(Ret (*fun) (A...)):built_well(true),fun(fun) {}
	Operation():fun([](A...) -> Ret{assert(false && "Operation built on sand!");}) {}

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
		assert(this);
		assert(built_well && "Calling operation constructed with default constructor is evil!");
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
		
		return fun(Store::tryCast(extract_robj_p(args))...);
	}
};

#define DOBODY1(decl,Name,args...)										\
	decl {																\
	return																\
		fold(mke<std::tuple<STORE_LIST> >(),							\
		[&](const auto &arg, const auto &accum){						\
		typedef decay<decltype(arg)> Store;								\
		typedef decltype(Name ## _impl(args)) ret_t;					\
		ret_t def;														\
		try {															\
		auto ret = tuple_cons(
                /*Name(args...);*/
#define DOBODY2(Name,args...) ,accum) ;									\
	assert(std::get<0>(ret).built_well);								\
	return ret;															\
	}																	\
	catch (Transaction::ClassCastException e){							\
		return tuple_cons(def,accum);									\
	}																	\
	},std::tuple<>());													\
	}
				
#define DECLARE_OPERATION2(Name, arg)								\
	DOBODY1(auto Name (arg a),Name,Store::tryCast(a))				\
	Name ## _impl(Store::tryCast(a))								\
	DOBODY2(Name,a)

#define DECLARE_OPERATION3(Name,Arg1, Arg2)								\
	DOBODY1(auto Name (Arg1 a, Arg2 b),Name,Store::tryCast(a),Store::tryCast(b)) \
	Name ## _impl (Store::tryCast(a),Store::tryCast(b))					\
	DOBODY2(Name,a,b)


#define DECLARE_OPERATION_IMPL2(count, ...) DECLARE_OPERATION ## count (__VA_ARGS__)
#define DECLARE_OPERATION_IMPL(count, ...) DECLARE_OPERATION_IMPL2(count, __VA_ARGS__)
#define DECLARE_OPERATION(...) DECLARE_OPERATION_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__)
