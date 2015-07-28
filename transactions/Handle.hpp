//new handle.  the old one is pretty broken.
#pragma once
#include "../BitSet.hpp"
#include "Basics.hpp"
#include "RemoteObject.hpp"
#include <memory>


struct HandleAbbrev{
	
	static constexpr std::true_type* CompatibleWithBitset = nullptr;
	const BitSet<HandleAbbrev>::member_t value;
	typedef decltype(value) itype;
	
	//dear programmer; it's on you to make sure that this is true.
	static constexpr int numbits = sizeof(decltype(value));
	
	operator decltype(value)() const {
		return value;
	}
	HandleAbbrev(decltype(value) v):value(v){}
	
	
	bool operator<(const HandleAbbrev& o) const {
		return value < o.value;
	}
	//idea; we use this for tracking the ReadSet.
};

template<Level l, HandleAccess HA>
struct GenericHandle {};

template<Level l, HandleAccess HA, typename T>
struct Handle : public GenericHandle<l,HA> {

private:
	const std::shared_ptr<RemoteObject<T> > _ro;
	Handle(std::shared_ptr<RemoteObject<T> > _ro):_ro(_ro){}
public:
	
	
	typename std::conditional<canWrite<HA>::value,
							  RemoteObject<T>&,
							  const RemoteObject<T>& >::type
	remote_object() const {
		assert(_ro);
		return *_ro;
	}

	Handle() {}
		
	static constexpr Level level = l;
	static constexpr HandleAccess ha = HA;
	typedef T stored_type;
	
	const T& get() const {
		assert(_ro);
		return _ro->get();
	}
	
	Handle clone() const {
		return *this;
	}
	
	void put(const T& t) {
		assert(_ro);
		_ro->put(t);
	}
	
	operator HandleAbbrev() const {
		HandleAbbrev::itype i = 1;
		return i << _ro->id;
	}
	
	HandleAbbrev abbrev() const {
		return *this;
	}

	template<Level lnew = l>
	Handle<lnew,HandleAccess::read,T> readOnly() const {
		static_assert(lnew == l || l == Level::strong,
					  "Error: request for strong read handle from causal base!");
		return Handle<lnew,HandleAccess::read,T>{_ro};
	}

	template<Level lnew = l>
	Handle<lnew,HandleAccess::write,T> writeOnly() const {
		static_assert(lnew == l || l == Level::causal,
					  "Error: request for causal write handle from strong base!");
		Handle<lnew,HandleAccess::write,T> r{_ro};
		return r;
	}

	template<Level l2, HandleAccess ha2, typename T2>
	friend struct Handle;

	template<template<typename> typename RO, typename... Args>
	static Handle<l,HA,T> make_handle(Args && ... ca){
		static_assert(std::is_base_of<RemoteObject<T>,RO<T> >::value,
					  "Error: must template on valid RemoteObject extender");
		RemoteObject<T> *rop = new RO<T>(std::forward<Args>(ca)...);
		std::shared_ptr<RemoteObject<T> > sp(rop);
		Handle<l,HA,T> ret(sp);
		return ret;
	}

};

template<Level l, HandleAccess HA, typename T,
		 template<typename> typename RO, typename... Args>
Handle<l,HA,T> make_handle(Args && ... ca)
{
	return Handle<l,HA,T>::template make_handle<RO, Args...>(std::forward<Args>(ca)...);
}

template<Level l, HandleAccess ha, typename T>
auto get_ReadSet(const Handle<l,ha,T> &h){
	return BitSet<HandleAbbrev>(h.abbrev());
}

template<Level l, HandleAccess ha, typename T>
std::ostream & operator<<(std::ostream &os, const Handle<l,ha,T>&){
	return os << "Handle<" << levelStr<l>() << ">";
}

template<typename T>
struct is_handle;

template<Level l, HandleAccess ha, typename T>
struct is_handle<Handle<l,ha,T> > : std::true_type {};

template<typename T>
struct is_handle : std::false_type {};

template<typename T>
struct is_not_handle : std::integral_constant<bool, !is_handle<T>::value >::type {};

template<typename H>
struct extract_type;

template<Level l, HandleAccess ha, typename T>
struct extract_type<Handle<l,ha,T> > {typedef T type;};

template<typename H>
struct extract_access;

template<Level l, HandleAccess ha, typename T>
struct extract_access<Handle<l,ha,T> > :
	std::integral_constant<HandleAccess, ha>::type {};


template<typename T>
struct is_readable_handle :
	std::integral_constant<bool,
						   is_handle<T>::value &&
						   canRead<extract_access<T>::value>::value >::type {};

template<typename T>
struct is_writeable_handle :
	std::integral_constant<bool,
						   is_handle<T>::value &&
						   canWrite<extract_access<T>::value>::value >::type {};
