#pragma once

#include "type_utils.hpp"
#include "tuple_extras.hpp"
#include "GDataStore.hpp"
#include "Store.hpp"
#include "SerializationSupport.hpp"

namespace myria{

	namespace mtl {
		struct Transaction;
	}

	namespace tracker {
		class Tracker;
	}

#define GeneralRemoteObject_body										\
	const int id = mutils::gensym();									\
	virtual bool ro_isValid(mtl::TransactionContext*) const = 0;		\
	virtual const DataStore<level>& store() const = 0;					\
	virtual DataStore<level>& store() = 0;								\
	virtual Name name() const = 0;										\
	virtual ~GeneralRemoteObject(){}									\

	template<Level>
	struct GeneralRemoteObject;

	template<>
	struct GeneralRemoteObject<Level::strong> : public mutils::ByteRepresentable{
		static constexpr Level level = Level::strong;
		GeneralRemoteObject_body
	};

	template<>
	struct GeneralRemoteObject<Level::causal> : public mutils::ByteRepresentable{
		static constexpr Level level = Level::causal;
		GeneralRemoteObject_body
		virtual const std::array<int,NUM_CAUSAL_GROUPS>& timestamp() const = 0;
		virtual std::vector<char> bytes() const = 0;
	};


	template<Level l2, HandleAccess ha2, typename T2> struct Handle;

	template<Level l, typename T>
	class RemoteObject : public GeneralRemoteObject<l>
	{
		//extend this plz!

		virtual const T& get(mtl::TransactionContext*) = 0;
		virtual void put(mtl::TransactionContext*,const T&) = 0;

		//TODO: delete these when you're done hacking around.
		RemoteObject(const RemoteObject&) = delete;

	public:
		RemoteObject(){}
		virtual ~RemoteObject(){}
		template<Level l2, HandleAccess HA, typename T2>
		friend struct Handle;

		using type = T;
		/*
		  template<HandleAccess ha, typename T2>
		  friend Handle<Level::strong,ha,T2>
		  run_ast_causal(CausalCache& cache, const CausalStore &s,
		  const Handle<Level::strong,ha,T2>& h);
		//*/
	
		static std::unique_ptr<RemoteObject> from_bytes(char const * _v); 


		friend struct mtl::Transaction;
		friend class tracker::Tracker;

	};

	template<typename T>
	struct get_ro_ptr_lvl;

	template<Level l, typename T>
	constexpr Level get_ro_ptr_lvl_f(RemoteObject<l,T>*){
		return l;
	}

	template<typename T>
	struct get_ro_ptr_lvl<T*> : std::integral_constant<Level, get_ro_ptr_lvl_f(mutils::mke_p<T>())>::type {};

	DecayTraits(get_ro_ptr_lvl);

	template<typename T>
	struct is_RemoteObj_ptr;

	template<template<typename> class C, typename T>
	struct is_RemoteObj_ptr<C<T>*> : std::integral_constant<
		bool,
		std::is_base_of<RemoteObject<Level::causal,T>,C<T> >::value
		|| std::is_base_of<RemoteObject<Level::strong,T>,C<T> >::value>::type {};

	template<template<Level,typename> class C, Level l, typename T>
	struct is_RemoteObj_ptr<C<l,T>*> : std::is_base_of<RemoteObject<l,T>,C<l,T> >::type {};

	template<Level l, typename T>
	struct is_RemoteObj_ptr_lvl : std::integral_constant<bool, is_RemoteObj_ptr<T>::value && get_ro_ptr_lvl<T>::value == l > {};

	template<template<Level, typename> class C, Level l, Level l2, typename T>
	struct is_RemoteObj_ptr_lvl<l, C<l2,T> > :
		std::integral_constant<
		bool, is_RemoteObj_ptr<C<l2,T> >::value && l == l2>::type {};

	DecayTraitsLevel(is_RemoteObj_ptr_lvl)
	DecayTraits(is_RemoteObj_ptr)

	template<typename T>
	struct is_RemoteObj_ptr : std::false_type{};

	template<typename T>
	struct is_not_RemoteObj_ptr : std::integral_constant<bool,!is_RemoteObj_ptr<T>::value >::type {};

	template<typename T>
	using cr_add = typename std::conditional<is_RemoteObj_ptr<T>::value, T, const std::decay_t<T>&>::type;

}

