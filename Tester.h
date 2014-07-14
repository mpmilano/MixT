#pragma once
#include "Backend.h"
#include <functional>
#include <list>
#include <utility>

namespace tester {
	
#if 0
	template<typename R, typename IR, backend::Level L, typename... A>
	class Fuzz {
	private:
		backend::DataStore<L> &ds;
		std::list<std::pair<std::function<IR (std::list<R>)> ,
				    std::function<R (backend::DataStore<L> &) > > >test_funs;
	public:
		Fuzz(backend::DataStore<L> &ds):ds(ds),test_funs(std::list<std::pair<std::function<IR (std::list<R>)> ,
				    std::function<R (backend::DataStore<L> &) > > >()){}
		void registerTestFunction(std::function<IR (std::list<R>)> &check_invariants,
					  std::function<R (backend::DataStore<L> &, A... )> &tf, 
					  typename std::add_lvalue_reference<typename std::add_const<A>::type>::type... extra_args){

			std::function<R (backend::DataStore<L> &) > &&storeargs = 
				[&](backend::DataStore<L> &ds) {return tf(ds, extra_args...); };
			test_funs.push_back(std::make_pair(check_invariants, storeargs));
		}
	};

	template<typename R, typename IR, backend::Level L, typename... A>
	Fuzz<R,IR,L,A> registerTestFunction(backend::DataStore<L> &ds,
				std::function<IR (std::list<R>)> &,
				std::function<R (backend::DataStore<L> &, A... )> &, 
					typename std::add_lvalue_reference<typename std::add_const<A>::type>::type ){
		return Fuzz<R,IR,L,A>(ds);
	}

#endif
	template<typename R, typename IR, backend::Level L, typename... A>
	IR registerTestFunction(backend::DataStore<L> &ds,
				std::function<IR (std::list<R>)> &check_invariants,
				std::function<R (backend::DataStore<L> &, A... )> &tf, 
					typename std::add_lvalue_reference<typename std::add_const<A>::type>::type... extra_args ){
		std::list<R> rlist;
		rlist.push_back(tf(ds,extra_args...));
		return check_invariants(rlist);
	}


}
