#pragma once
#include "Backend.hpp"
#include "Instance.hpp"

template<location l>
class Instance<l>::GenericHandle {
public:
	typedef int HandleID;
	virtual HandleID id() const = 0;
};


template<location l1>
template<typename T>
class Instance<l1>::TypedHandle : GenericHandle{
private:
	StoredObject<T> &obj;
public:
	TypedHandle(StoredObject<T> &o):obj(o){}
	
};

template<location l1>
template<typename T, Level l, Access::Access a>
class Instance<l1>::Handle : TypedHandle<T>{
	
public:
	Handle(StoredObject<T> &o):TypedHandle<T>(o){}
	
};
