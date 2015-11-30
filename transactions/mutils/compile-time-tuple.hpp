#pragma once
#include <type_traits>
#include <tuple>
#include "restrict.hpp"
#include "utils.hpp"

namespace mutils{

	namespace ct {

		template<typename... B>
		struct tuple;

		template<int i, typename A, typename... B>
		constexpr auto get(const typename std::enable_if<(i > 0) && (sizeof...(B) > 0), tuple<A,B...> >::type &t){
			return get<i-1>(t.rest);
		}

		template<int i, typename A, typename... B>
		constexpr typename std::enable_if<i == 0 && (sizeof...(B) > 0), A>::type
																		   get(const tuple<A,B...> &t){
			return t.a;
		}

		template<int i, typename A, restrict(i == 0)>
		constexpr auto get(const tuple<A> &t){
			return t.a;
		}

		template<typename A, typename... B>
		struct tuple<A,B...> {
			const A a;
			const tuple<B...> rest;

			template<int i>
			constexpr auto get() const {
				static_assert(i < (sizeof...(B) + 1),"Error: index out of range!");
				return ct::get<i,A,B...>(*this);
			}

			std::tuple<A,B...> to_std_tuple(){
				return std::tuple_cat(std::make_tuple(a),rest.to_std_tuple());
			}

			const int size = sizeof...(B) + 1;
		};
		template<>
		struct tuple<> {
		
			template<int i>
			constexpr int get(){
				static_assert(i > -17,"Error: index out of range!");
				return 0;
			}

			std::tuple<> to_std_tuple(){
				return std::tuple<>();
			}
		};

		template<typename A>
		struct tuple<A> {
			const A a;
			tuple<> rest;
			template<int i = 0, restrict(i == 0)>
			constexpr A get(){
				static_assert(i == 0,"Error: index out of range!");
				return a;
			}

			std::tuple<A> to_std_tuple(){
				return std::make_tuple(a);
			}
		};


		template<typename A>
		constexpr auto make_tuple(const A &a) {
			return tuple<A>{a};
		}
	
		template<typename A, typename ... B>
		constexpr typename std::enable_if<(sizeof...(B) > 0),tuple<A,B...> >::type
		make_tuple(const A &a, const B & ... b){
			return tuple<A,B...>{a, make_tuple(b...)};
		}
	
		template<typename A, typename... B>
		constexpr typename std::enable_if<sizeof...(B) != 0, tuple<A,B...> >::type
		cons(A a, tuple<B...> b){
			return tuple<A,B...>{a,b};
		}

		template<typename A>
		constexpr auto cons(A a, tuple<>){
			return tuple<A>{a};
		}

	}


}
