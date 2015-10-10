#pragma once
#include "Temporary.hpp"

struct Assignment {
	
};

template<unsigned long long ID, Level l, typename T, typename Temp, typename E,
		 restrict((std::is_same<Temporary<ID,l,T>, Temp>::value))>
auto operator<<(const RefTemporary<ID,l,T,Temp>&, const E &){
	return Assignment{};
}
