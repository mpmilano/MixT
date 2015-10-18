#pragma once
#include "RefTemporary.hpp"
#include "MutableTemporary.hpp"
#include "Temporary.hpp"

#define MutAssigner(c) MutCreator<unique_id<(sizeof(c) / sizeof(char)) - 1>(c)>{c}
#define ImmutAssigner(c) ImmutCreator<unique_id<(sizeof(c) / sizeof(char)) - 1>(c)>{c}

template<unsigned long long ID>
struct MutCreator {
	const std::string &name;

	template<typename T>
	auto operator=(const T& t_) const {
		static_assert(is_ConExpr<T>::value, "Error: cannot assign non-expression");
		auto t = wrap_constants(t_);
		static constexpr Level l = get_level<T>::value;
		RefTemporary<ID,l,decltype(t),MutableTemporary<ID,l,decltype(t) > >
			rt(MutableTemporary<ID,l,decltype(t) >(name,t));
		return rt;
	}
};

template<unsigned long long ID>
struct ImmutCreator {
	const std::string &name;

	template<typename T>
	auto operator=(const T& t_) const {
		static_assert(is_ConExpr<T>::value, "Error: cannot assign non-expression");
		auto t = wrap_constants(t_);
		static constexpr Level l = get_level<T>::value;
		RefTemporary<ID,l,decltype(t),Temporary<ID,l,decltype(t) > >
			rt(Temporary<ID,l,decltype(t) >(name,t));
		return rt;
	}

};

