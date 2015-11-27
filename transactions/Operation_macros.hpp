#pragma once
#include "macro_utils.hpp"
#include "restrict.hpp"

#define DECLARE_OPERATION(Name, arg...) static NoOperation Name ## _impl(arg){return NoOperation();}



#define DOBODY1(decl,Name,args...)										\
	decl {																\
	return																\
		mutils::fold(*mutils::mke_p<std::tuple<STORE_LIST> >(),			\
			 [&](const auto &arg, const auto &accum){					\
				 typedef std::decay_t<decltype(arg)> Store;				\
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
				 catch (mtl::Transaction::ClassCastException e){		\
					 return tuple_cons(def,accum);						\
				 }														\
				 },std::tuple<>());										\
	}
				
#define FINALIZE_OPERATION1(Name, arg)								\
	DOBODY1(auto Name arg,Name,Store::tryCast(a)) \
	Store::Name ## _impl(Store::tryCast(a))							\
	DOBODY2(Name,a)

	

#define FINALIZE_OPERATION2(Name,args)								\
	DOBODY1(auto Name args,Name,Store::tryCast(a),Store::tryCast(b)) \
	Store::Name ## _impl (Store::tryCast(a),Store::tryCast(b))		 \
	DOBODY2(Name,a,b)


#define FINALIZE_OPERATION(Name,count,args) FINALIZE_OPERATION ## count (Name,args)
