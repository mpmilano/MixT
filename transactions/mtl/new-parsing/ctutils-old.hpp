#pragma once

#include <vector>
#include <type_traits>
#include <iostream>
#include "union.hpp"


using test_union = Union<int,std::size_t,double>;

template<typename target, typename... t> struct is_in;

template<typename target> struct is_in<target> : public std::false_type {};

template<typename target, typename cand, typename... t>
struct is_in<target,cand,t...> :
	public std::integral_constant<
	bool,
	std::is_same<target,cand>::value ||
	is_in<target,t...>::value
	>{};

template<typename target, typename... t>
using if_in_t = std::enable_if_t<is_in<target,t...>::value,target>;

template<typename T>
struct Option{
	
	Union<std::nullptr_t, T> internal;
	constexpr Option(T t):internal{t}{}
	constexpr Option():internal(nullptr){}
	
	template<typename F, typename Accum>
	constexpr Accum* fold(F &&f, Accum* accum);

	template<typename F, typename Accum>
	constexpr Accum* fold(F &&f, Accum* accum) const;
	template<typename F>
	constexpr auto match(F &&f){
		switch (internal.which()){
		case 0: return f(internal.template get<0>());
		case 1: return f(&internal.template get<1>());
		default: throw "impossible";
		}
	}
	
	template<typename F>
	constexpr auto match(F &&f) const {
		switch (internal.which()){
		case 0: return f(internal.template get<0>());
		case 1: return f(&internal.template get<1>());
		default: throw "impossible";
		}
	}

	constexpr bool is_null() const {
		//assert(internal.is_initialized);
		//assert(internal.which() < 2);
		return internal.which() == 0;
	}

	constexpr auto operator=(const Option &t2){
		internal.operator=(t2.internal);
		return *this;
	}

	constexpr auto operator=(const T &t2){
		internal.operator=(t2);
		return *this;
	}
	
};

template<std::size_t length>
struct ctstring{
	constexpr ctstring(){}
	char data[length] = {0};

	constexpr ctstring& operator=(const ctstring& o){
		for (auto i = 0u; i < length; ++i){
			data[i] = o.data[i];
		}
		return *this;
	}
};

using string = ctstring<2048>;


struct result {
	enum class status{
		ok,error
			};
	constexpr result(status s)
		:status(s){}
	constexpr result()
		:status(status::ok){}
	status status;
	Option<string> message;
	
};

auto& operator<<(std::ostream& o, result r){
	switch(r.status){
	case result::status::ok :
		return o << "ok";
	case result::status::error:
		return o << "error";
	}
}

constexpr result ok_result(){
	return result{result::status::ok};
}

template<std::size_t size>
constexpr result err_result(const char (&)[size]){
	return result{result::status::error};
}

template<typename T>
template<typename F, typename Accum>
constexpr Accum* Option<T>::fold(F &&f, Accum* accum){
	if (internal.which() == 1) return f(internal.template get<1>(),
																			accum);
	else return accum;
};

template<typename T>
template<typename F, typename Accum>
constexpr Accum* Option<T>::fold(F &&f, Accum* accum) const {
	if (internal.which() == 1)
		return f(internal.template get<1>(),accum);
	else return accum;
};
