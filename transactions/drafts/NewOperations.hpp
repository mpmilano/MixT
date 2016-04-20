#pragma once
#include <type_traits>
#include <memory>
#include <iostream>

enum class RegisteredOperations {
	Increment
};

enum class HandleAccess{
	all
};

enum class Level {
	single
};


template<Level l, typename T>
struct RemoteObject {
};

struct SelfType{};

template<RegisteredOperations Name>
struct OperationIdentifier_struct{};

template<RegisteredOperations Name>
using OperationIdentifier = OperationIdentifier_struct<Name>*;

template<typename Handle>
struct convert_SelfType{
	
	template<typename candidate>
	using act = std::conditional_t<
		std::is_same<candidate,SelfType>::value,
		Handle,
		candidate>;
};


template<RegisteredOperations Name, typename... Args>
struct SupportedOperation {

	template<typename Handle>
	struct SupportsOn{
	
		struct operation_super {
			virtual void act(typename convert_SelfType<Handle&>::template act<Args>...) = 0;
			virtual ~operation_super(){}
		};

		template<typename DataStore, template<typename> class RO>
		struct operation_impl : public operation_super{
			DataStore &ds;
			
			static constexpr RO<typename Handle::storedType>& reduce_selfTypes(SelfType*, Handle& h){
				return static_cast<RO<typename Handle::storedType>& >(*h._ro);
			}
			
			template<typename Arg>
			static constexpr auto& reduce_selfTypes(Arg*, Arg& a){
				static_assert(!std::is_same<Arg,SelfType>::value,"Error: overload resolution failed!");
				return a;
			}
			
			operation_impl(DataStore &ds):ds(ds){}
			
			void act(typename convert_SelfType<Handle&>::template act<Args>... a){
				ds.operation(OperationIdentifier<Name>{nullptr},
							 this->template reduce_selfTypes(((Args*)nullptr), a)...);
			}
		};

		using operation = std::shared_ptr<operation_super>;
		
		operation op;

		SupportsOn(operation op):op(op){}
		
		template<template<typename> class RemoteObject, typename DataStore>
		static operation wrap_operation(DataStore& ds){
			return operation{
				new operation_impl<DataStore,RemoteObject>(ds)};
		}

		virtual Handle& downCast() = 0;
		SupportsOn& upCast(OperationIdentifier<Name>){
			return *this;
		}
	};
};


template<Level l, HandleAccess HA, typename T, typename... SupportedOperations>
struct Handle :
	public SupportedOperations::template SupportsOn<Handle<l,HA,T,SupportedOperations...> >...
{
	using ThisRemoteObject = RemoteObject<l,T>;
	using storedType = T;
	std::shared_ptr<RemoteObject<l,T> > _ro;
	

	template<typename DataStore, template<typename> class RO>
		Handle(std::shared_ptr<RO<T> > ro, DataStore& ds)
		:SupportedOperations::template SupportsOn<Handle>(
			SupportedOperations::template SupportsOn<Handle>::template wrap_operation<RO>(ds))...
		,_ro(ro)
	{}

	Handle& downCast() { return *this;}
};

class FakeStore {
public:
	
	template<typename T>
	struct Object : RemoteObject<Level::single,T> {};

	static constexpr auto Increment = RegisteredOperations::Increment;

	template<typename T>
	void operation(OperationIdentifier<Increment>, Object<T> &o){
		std::cout << "Look, I did an operation!" << std::endl;
	}

	template<typename T>
	using FakeHandle =
		Handle<Level::single, HandleAccess::all, T, SupportedOperation<Increment, SelfType> >;
		
	template<typename T>
	FakeHandle<T> newObject(){
		return FakeHandle<T>(std::make_shared<Object<T> >(),*this);
	}
	
};

template<RegisteredOperations Name, typename Handle, typename... Args>
auto do_op_3(typename SupportedOperation<Name,SelfType,Args...>::template SupportsOn<Handle> &hndl,
				typename convert_SelfType<Handle&>::template act<Args>... args){
	
	return hndl.op->act(hndl.downCast(),args...);
}

template<RegisteredOperations Name, typename Handle, typename... Args>
auto do_op_2(Handle &h, Args && ... args){
	constexpr OperationIdentifier<Name> op{nullptr};
	return do_op_3<Name,Handle>(h.upCast(op),std::forward<Args>(args)...);
}

template<RegisteredOperations Name, typename... Args>
auto do_op(Args && ... args){
	struct Operate{
		auto call(){
			ignore(call_all_strong(args)...);
			ignore(call_all_causal(args)...);
			return do_op<Name>(cached(args)...);
		}
	};
}
