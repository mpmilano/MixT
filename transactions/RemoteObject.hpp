#pragma once

#include "type_utils.hpp"

struct GeneralRemoteObject {
	const int id = gensym();
};

template<typename T>
class RemoteObject : public GeneralRemoteObject {
	//extend this plz!

	virtual const T& get() const = 0;
	virtual void put(const T&) = 0;
	virtual bool isValid() const = 0;


	//TODO: delete these when you're done hacking around.
	RemoteObject(const RemoteObject&) = delete;

public:
	RemoteObject(){}
	template<Level l, HandleAccess HA, typename T2>
	friend struct Handle;

};


template<typename T>
struct is_RemoteObj_ptr;

template<template<typename> class C, typename T>
struct is_RemoteObj_ptr<C<T>*> : std::is_base_of<RemoteObject<T>,C<T> >::type {};

template<template<typename> class C, typename T>
struct is_RemoteObj_ptr<const C<T>*> : std::is_base_of<RemoteObject<T>,C<T> >::type {};

template<typename T>
struct is_RemoteObj_ptr : std::false_type{};

template<typename T>
struct is_not_RemoteObj_ptr : std::integral_constant<bool,!is_RemoteObj_ptr<T>::value >::type {};

template<typename T>
using cr_add = typename std::conditional<is_RemoteObj_ptr<T>::value, T, const decay<T>&>::type;

template<typename T>
using generalize_ro = typename std::conditional<is_RemoteObj_ptr<T>::value, const GeneralRemoteObject*,  T>::type;
