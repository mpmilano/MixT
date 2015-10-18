#pragma once
#include "Temporary.hpp"

template<unsigned long long ID, Level l, typename T>
struct MutableTemporary : public Temporary<ID, l,T> {
	MutableTemporary(const std::string& name, const T& t):
		Temporary<ID,l,T>(name,t){}

	typedef typename std::integral_constant<Level,l>::type level;
	typedef T type;
	typedef std::true_type found;
	typedef typename std::integral_constant<unsigned long long, ID>::type key;


};

template<unsigned long long ID, Level l, typename T>
auto find_usage(const MutableTemporary<ID,l,T> &rt){
	return shared_copy(rt);
}

template<unsigned long long ID, unsigned long long ID2, Level l, typename T>
struct contains_temporary<ID, MutableTemporary<ID2,l,T> > : std::integral_constant<bool, ID == ID2> {};


template<unsigned long long ID, unsigned long long ID2, Level l, typename T>
std::enable_if_t<ID != ID2,std::nullptr_t> find_usage(const MutableTemporary<ID2,l,T> &rt){
	return nullptr;
}
