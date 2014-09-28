#pragma once
#include "Backend.hpp"
#include "Instance.hpp"

template<location l>
Instance<l>::class GenericHandle {
public:
	typedef int HandleID;
	const HandleID id() = 0;
	
	template<typename T, Level l2, Access::Access a>
	bool is_type(Handle<T,l2,a>*) = 0;

};



template<location l1>
template<typename T>
Instance<l1>::class TypedHandle : GenericHandle{
private:
	StoredObject<T> &obj;
};

template<location l1>
template<typename T, Level l, Access::Access a>
Instance<l1>::class Handle : TypedHandle<T>{
	
};
