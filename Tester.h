#pragma once
#include "Backend.h"
#include <functional>
#include <list>
#include <utility>
#include <tuple>
#include <memory>

#define CONST_LVALUE(x)  typename add_lvalue_reference<typename add_const<x>::type>::type
#define RVALUE(x) typename add_rvalue_reference<x>::type


namespace tester {
	using namespace backend;
	using namespace std;

	template<typename R, typename IR, Level L, typename... A>
	class Fuzz;

	template<Level L, typename R, typename IR,  typename... A>
	Fuzz<R,IR,L,A...> registerTestFunction(DataStore<L> &,
					       function<IR (list<R>)> &,
					       function<R (DataStore<L> &, A... )> &,
					       CONST_LVALUE(A)... extra_args);
		
	template<typename R, typename IR, Level L, typename... A>
	class Fuzz {
	public:
		typedef function<IR (list<R>)> checker_fun;
		typedef function<R (DataStore<L> &) > test_fun;
		typedef list<pair<checker_fun, test_fun>> test_list;
	private:
		DataStore<L> &ds;
		test_list test_funs;
	public:
		Fuzz(DataStore<L> &ds):	ds(ds), test_funs(test_list()){}


		void registerTestFunction(checker_fun &check_invariants, 
					  function<R (DataStore<L> &, A... )> &tf, 
					  CONST_LVALUE(A)... extra_args){
			test_fun &&storeargs = bind(tf, placeholders::_1, ref(extra_args)...);
			test_funs.push_back(make_pair(check_invariants, storeargs));
		}

	};

	template<Level L, typename R, typename IR,  typename... A>
	Fuzz<R,IR,L,A...> registerTestFunction(DataStore<L> &ds,
					       function<IR (list<R>)> &check_invariants,
					       function<R (DataStore<L> &, A... )> &tf,
					       CONST_LVALUE(A)... extra_args){
		auto &&ret = Fuzz<R,IR,L,A...>(ds);
		ret.registerTestFunction(check_invariants, tf, extra_args...);
		return ret;
	}
}
