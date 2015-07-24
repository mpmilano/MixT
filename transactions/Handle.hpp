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

	const std::shared_ptr<const RemoteObject<T> > ro;
	
	static constexpr Level level = l;
	static constexpr HandleAccess ha = HA;
	typedef T stored_type;
	const T& get() const {
		return ro->get();
	}
	
	Handle clone() const {
		return *this;
	}
	
	void put(const T& t) {
		ro->put(t);
	}
	
	operator HandleAbbrev() const {
		assert(false && "unimplemented");
	}
	
	HandleAbbrev abbrev() const {
		return *this;
	}

};

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
