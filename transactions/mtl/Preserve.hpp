#pragma once
#include "Basics.hpp"

namespace myria{

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
		return mtl::get_level<T>::value;
	}

	template<typename T>
	auto env_expr_helper_2(const Preserve<T>& t){
		return mtl::environment_expressions(t.t);
	}

	template<typename C, typename T>
	auto extract_robj_p(mtl::TransactionContext &ctx, const Preserve2<C,T> &t){
		return cached(t.c,t.t);
	}
}
