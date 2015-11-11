#pragma once

#include "Handle.hpp"
#include "tuple_extras.hpp"
#include "utils.hpp"
#include "args-finder.hpp"
#include "ConStatement.hpp"
#include "filter-varargs.hpp"
#include "type_utils.hpp"
#include "ConExpr.hpp"
#include "Operation_macros.hpp"

//because we don't have a lot of inspection abilities into this function,
//we only allow write-enabled handles into which *all* read-enabled handles are
//permitted to flow.


//Idea: you can have template<...> above this and it will work!

#define OPERATION(Name, args...) static auto Name ## _impl (args) { \
	struct r { static bool f(args)

#define END_OPERATION };								\
	auto fp = r::f;										\
	return Operation<extract_store<decltype(fp)>,decltype(fp)>(fp);	\
	}

template<typename, typename>
struct Operation;

template<typename T>
struct Preserve{
	const T t;
};

template<typename C, typename T>
struct Preserve2 {
	const C c;
	const T t;
};

template<typename T>
struct is_preserve : std::false_type {};

template<typename T>
struct is_preserve<Preserve<T> > : std::true_type {};

template<typename C, typename T>
struct is_preserve<Preserve2<C,T> > : std::true_type {};

DecayTraits(is_preserve)

template<typename T>
auto preserve(const T &t){
	return Preserve<T>{t};
}

template<typename C, typename T>
auto cached(const C &c, const Preserve<T> &t){
	return Preserve2<const C&,T>{c,t.t};
}

template<typename T>
constexpr Level get_level_dref(Preserve<T> const * const){
	//TODO: should the semantics of "preserve" be such that
	//we should recur here? 
	return get_level<T>::value;
}

template<typename T>
auto handles_helper_2(const Preserve<T>& t){
	return ::handles(t.t);
}

template<typename C, typename T>
auto extract_robj_p(const Preserve2<C,T> &t){
	return cached(t.c,t.t);
}

template<typename T, restrict(!is_handle<decay<T> >::value && !is_preserve<T>::value )>
 auto extract_robj_p(T&& t) {
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
	
	Operation(Ret (*fun) (A...)):built_well(true),fun(fun) {
	}
	
	Operation():fun([](A...) -> Ret{assert(false && "Operation built on sand!");}) {
	}

	Operation(const Operation<Store, Ret(*) (A...) >& op):built_well(op.built_well),fun(op.fun){
	}

	template<typename ___T, typename Acc>
	using type_check = std::pair<Rest<Left<Acc> >,
								 std::integral_constant
								 <bool,
								  sassert2(___T, First<Left<Acc> >, 
										   (std::is_same<decay<___T>, decay<First<Left<Acc> > > >::value || 
											(is_handle<decay<___T> >::value &&
											 is_RemoteObj_ptr<First<Left<Acc> > >::value)
											|| (is_preserve<___T>::value && is_handle<decay<First<Left<Acc> > > >::value) )) && 
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
		
		//do this here so we abort before causal tracking happens
		discard(Store::tryCast(extract_robj_p(args))...);			

		auto h_read = filter_tpl<is_readable_handle>(std::make_tuple(args...));
		auto h_strong_read = filter_tpl<is_strong_handle>(h_read);
		auto h_causal_read = filter_tpl<is_causal_handle>(h_read);
		auto h_write = filter_tpl<is_writeable_handle>(std::make_tuple(args...));
		auto h_strong_write = filter_tpl<is_strong_handle>(h_write);
		auto h_causal_write = filter_tpl<is_causal_handle>(h_write);
		foreach(h_strong_read,
				[](const auto &h){h.tracker.onRead(h.store(),h.name());});
		foreach(h_causal_read,
				[](const auto &h){h.tracker.onRead(h.store(),h.name(),h.remote_object().timestamp());});

		auto &&ret = fun(Store::tryCast(extract_robj_p(args))...);
		foreach(h_strong_write, [](const auto &h){h.tracker.onWrite(h.store(),h.name());});
		foreach(h_causal_write, [](const auto &h){h.tracker.onWrite(h.store(),h.name(),h.remote_object().timestamp());});
		return ret;
	}
};

struct OperationNotFoundError {};

struct NoOperation {
	static constexpr bool built_well = false;

	typedef NoOperation F;

	template<typename... Args>
	bool operator()(Args && ...) const {
		throw OperationNotFoundError ();
		return false;
	}
};



#define op_arg(x...) extract_robj_p(x)
