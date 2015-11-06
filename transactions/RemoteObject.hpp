#pragma once

#include "type_utils.hpp"
#include "tuple_extras.hpp"
#include "GDataStore.hpp"
#include "Store.hpp"
#include "SerializationSupport.hpp"


template<Level l>
struct GeneralRemoteObject : public ByteRepresentable{
	const int id = gensym();
	virtual void setTransactionContext(TransactionContext*) = 0;
	virtual TransactionContext* currentTransactionContext() = 0;
	virtual bool ro_isValid() const = 0;
	virtual const GDataStore& store() const = 0;
	virtual GDataStore& store() = 0;
	virtual int name() const = 0;
	virtual ~GeneralRemoteObject(){}
};

template<Level l2, HandleAccess ha2, typename T2> struct Handle;
class Tracker;

template<Level l, typename T>
class RemoteObject : public GeneralRemoteObject<l>
{
	//extend this plz!

	virtual const T& get(Tracker *t) = 0;
	virtual void put(const T&) = 0;

	//TODO: delete these when you're done hacking around.
	RemoteObject(const RemoteObject&) = delete;

public:
	RemoteObject(){}
	virtual ~RemoteObject(){}
	template<Level l, HandleAccess HA, typename T2>
	friend struct Handle;

	using type = T;
/*
	template<HandleAccess ha, typename T2>
	friend Handle<Level::strong,ha,T2>
	run_ast_causal(CausalCache& cache, const CausalStore &s,
				   const Handle<Level::strong,ha,T2>& h);
//*/
	
	template<typename Arg, typename Accum>
	using Pointerize = Cons_t<Arg*,Accum>;
	
	static std::unique_ptr<RemoteObject> from_bytes(char* _v); 


	friend struct Transaction;

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
