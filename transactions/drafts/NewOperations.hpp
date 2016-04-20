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
	struct operation_super {
		virtual void act(typename convert_SelfType<Handle&>::template act<Args>...) = 0;
		virtual ~operation_super(){}
	};

	template<typename Handle, typename DataStore, template<typename> class RO>
	struct operation_impl : public operation_super<Handle>{
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

	template<typename Handle>
	using operation = std::shared_ptr<operation_super<Handle> >;


	template<typename Handle, template<typename> class RemoteObject, typename DataStore>
	static operation<Handle> wrap_operation(DataStore& ds){
		return operation<Handle>{new operation_impl<Handle,DataStore,RemoteObject>(ds)};
	}
};

template<Level l, HandleAccess HA, typename T, typename SupportedOperation>
struct Handle {
	using ThisRemoteObject = RemoteObject<l,T>;
	using storedType = T;
	std::shared_ptr<RemoteObject<l,T> > _ro;
	typename SupportedOperation::template operation<Handle> op;

	template<typename DataStore, template<typename> class RO>
	Handle(std::shared_ptr<RO<T> > ro, DataStore& ds)
		:_ro(ro),op(SupportedOperation::template wrap_operation<Handle,RO>(ds)){}
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

struct Operation {
	std::function<void ()> act;
};


template<RegisteredOperations Name, typename T, HandleAccess Ha, Level l, typename... Args>
static Operation do_op(Handle<l,Ha,T,SupportedOperation<Name,SelfType,Args...> > &hndl,
					typename convert_SelfType<Handle<l,Ha,T,SupportedOperation<Name,SelfType,Args...> >&>::template act<Args>... args){
	return Operation{[=]() mutable {hndl.op->act(hndl,args...);}};
}
