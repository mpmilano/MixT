#pragma once



template<typename T>
class RemoteObject {
	//extend this plz!

	virtual T& get() const = 0;
	virtual void put(const T&) const = 0;



	//TODO: delete these when you're done hacking around.
	RemoteObject() = delete;
	RemoteObject(const RemoteObject&) = delete;

	template<Level l, HandleAccess HA, typename T2>
	friend struct Handle;

};

template<typename T>
struct is_RemoteObj_ptr;

template<template<typename> class C, typename T>
struct is_RemoteObj_ptr<C<T>*> : std::is_base_of<RemoteObject<T>,C<T> >::type {};

template<typename T>
struct is_RemoteObj_ptr : std::false_type{};

template<typename T>
struct is_not_RemoteObj_ptr : std::integral_constant<bool,!is_RemoteObj_ptr<T>::value >::type {};
