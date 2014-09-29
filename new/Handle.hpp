#pragma once
#include "Backend.hpp"
#include "Instance.hpp"
#include "StoredObject.hpp"

namespace hidden_internal_stuff_handle {

	template<typename Maybe>
	struct is_H_helper{
		template<location l, typename T>
		static constexpr bool test(typename Instance<l>::template TypedHandle<T>* ){return true;}
		template<typename _t>
		static constexpr bool test(_t ){
			static_assert(! std::is_same<_t, Instance<1>::TypedHandle<int> >::value,"what is here?"); 
			return false;}
		static constexpr bool value = test( (Maybe*) nullptr);
		
	};
}

template<location l>
template<typename Maybe>
struct Instance<l>::is_handle : 
	public std::integral_constant 
<bool, hidden_internal_stuff_handle::is_H_helper<Maybe>::value > {};

//static_assert(Instance<0>::is_handle<Instance<0>::TypedHandle<int > >::value, "this is very frustrating.");

template<location l>
class Instance<l>::GenericHandle {
public:
	virtual typename StoredBlob::ObjectID id() const = 0;
};


template<location l1>
template<typename T>
class Instance<l1>::TypedHandle : GenericHandle{
private:
	StoredObject<T> &obj;
public:
	TypedHandle(StoredObject<T> &o):obj(o){}
	virtual typename StoredBlob::ObjectID id() const {
		return obj.id();
	}
	friend class LogStore;
	
};

template<location l1>
template<typename T, Level l, Access::Access a>
class Instance<l1>::Handle : TypedHandle<T>{
	
public:
	Handle(StoredObject<T> &o):TypedHandle<T>(o){}
	
};
