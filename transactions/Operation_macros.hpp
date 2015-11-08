#pragma once
#include "macro_utils.hpp"
#include "restrict.hpp"

#define DECLARE_OPERATION(Name, arg...) static NoOperation Name ## _impl(arg){return NoOperation();}



#define DOBODY1(decl,Name,args...)										\
	decl {																\
	return																\
		fold(*mke_p<std::tuple<STORE_LIST> >(),							\
			 [&](const auto &arg, const auto &accum){					\
				 typedef decay<decltype(arg)> Store;					\
				 typedef decltype(Store::Name ## _impl(args)) ret_t;	\
				 ret_t def;												\
				 try {													\
					 auto ret = tuple_cons(
/*Name(args...);*/
#define DOBODY2(Name,args...) ,accum) ;									\
					 assert(std::get<0>(ret).built_well &&				\
							"Did you actually implement this operation?"); \
					 return ret;										\
					 }													\
				 catch (Transaction::ClassCastException e){				\
					 return tuple_cons(def,accum);						\
				 }														\
				 },std::tuple<>());										\
	}
				
#define FINALIZE_OPERATION2(Name, arg)								\
	DOBODY1(auto Name (typename argument_type<void (arg)>::type a),Name,Store::tryCast(a)) \
	Store::Name ## _impl(Store::tryCast(a))							\
	DOBODY2(Name,a)

	

#define FINALIZE_OPERATION3(Name,Arg1, Arg2)								\
	DOBODY1(auto Name (Arg1 a, Arg2 b),Name,Store::tryCast(a),Store::tryCast(b)) \
	Store::Name ## _impl (Store::tryCast(a),Store::tryCast(b))			\
	DOBODY2(Name,a,b)


#define FINALIZE_OPERATION_IMPL2(count, ...) FINALIZE_OPERATION ## count (__VA_ARGS__)
#define FINALIZE_OPERATION_IMPL(count, ...) FINALIZE_OPERATION_IMPL2(count, __VA_ARGS__)
#define FINALIZE_OPERATION(...) FINALIZE_OPERATION_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__)
