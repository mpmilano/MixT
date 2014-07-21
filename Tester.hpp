#pragma once
#include "Backend.hpp"
#include <functional>
#include <list>
#include <utility>
#include <tuple>
#include <memory>
#include <cstdlib>
#include <vector>

#define CONST_LVALUE(x)  typename add_lvalue_reference<typename add_const<x>::type>::type
#define RVALUE(x) typename add_rvalue_reference<x>::type


namespace tester {
	using namespace backend;
	using namespace std;

	//Fuzzer: register unit tests to run

	//The idea is that the "fastest" level is replaced with L. 
	//This lets you try out operations at different consistencies.
	template<Level L, typename R, typename IR>
	class Fuzz;

	template<Level L = Level::fastest, typename R, typename IR,  typename... A>
	Fuzz<L, R,IR> registerTestFunction(DataStore &,
					       function<IR (list<R>)> &,
					       function<R (DataStore &, A... )> &,
					       CONST_LVALUE(A)... extra_args);
		
	template<Level L = Level::fastest, typename R, typename IR>
	class Fuzz {
	public:
		typedef function<IR (list<R>)> checker_fun;
		typedef function<R (DataStore &) > test_fun;
		typedef list<pair<checker_fun, test_fun>> test_list;
	private:
		DataStore &ds;
		test_list test_funs;
		Fuzz(DataStore &ds):	ds(ds), test_funs(test_list()){}
	public:
		Fuzz(const Fuzz<L,R,IR> &) = delete;
		Fuzz(Fuzz<L,R,IR> &&fz):ds(fz.ds),test_funs(std::move(fz.test_funs)){}
		
		template <typename... A>
		void registerTestFunction(checker_fun &check_invariants, 
					  function<R (DataStore &, A... )> &tf, 
					  CONST_LVALUE(A)... extra_args){
			test_funs.push_back(
				make_pair(check_invariants, 
					  bind(tf, placeholders::_1, ref(extra_args)...)));
		}

		std::list<IR> runTestFunctions(){
			auto old_level = ds.fastest_lvl; 
			ds.fastest_lvl = L;
			std::list<IR> &&test_res = std::list<IR>();
			for (auto &pair : this->test_funs) {
				auto &checker = pair.first;
				auto &tester = pair.second;
				int random = rand() %100; 
				std::list<R> results_list;
				for (int i = 0; i < random; ++i)
					results_list.push_back(tester(ds));
				test_res.push_back(checker(results_list));
			}
			ds.fastest_lvl = old_level;
			return test_res;
		}

		template<Level L1, typename R1, typename IR1,  typename... A1>
		friend Fuzz<L1,R1,IR1> registerTestFunction(DataStore &,
							    function<IR1 (list<R1>)> &,
							    function<R1 (DataStore &, A1... )> &,
							    CONST_LVALUE(A1)... extra_args);
	};

	template<Level L = Level::fastest, typename R, typename IR,  typename... A>
	Fuzz<L,R,IR> registerTestFunction(DataStore &ds,
					       function<IR (list<R>)> &check_invariants,
					       function<R (DataStore &, A... )> &tf,
					       CONST_LVALUE(A)... extra_args){
		static bool ranit = false;
		if (ranit) {throw "Run only once!";}
		ranit = true;
		auto &&ret = Fuzz<L,R,IR>(ds);
		ret.registerTestFunction(check_invariants, tf, extra_args...);
		return std::move(ret);
	}
}
