#pragma once

struct wrapper_factor
{
	template<class F>
	constexpr wrapper<F> operator += (F*)
		{
			return {};
		}
};

struct addr_add
{
	template<class T>
	friend typename std::remove_reference<T>::type *operator+(addr_add, T &&t)
		{
			return &t;
		}
};

#define STATIC_LAMBDA wrapper_factor() += true ? nullptr : addr_add() + []

