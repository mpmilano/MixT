#pragma once
#include "Basics.hpp"
#include "RemoteObject.hpp"

namespace myria{

	namespace mtl{
		template<typename>
		struct get_level;
		template<typename>
		struct extract_type;
	}
	
enum class RegisteredOperations {
	Increment, Insert, Clone
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
			virtual void act(mtl::TransactionContext* _ctx,typename convert_SelfType<Handle&>::template act<Args>...) = 0;
			virtual ~operation_super(){}
		};

		template<typename DataStore, template<typename> class RO>
		struct operation_impl : public operation_super{
			DataStore &ds;
			
			static constexpr RO<typename Handle::stored_type>& reduce_selfTypes(SelfType*, Handle& h){

				return static_cast<RO<typename Handle::stored_type>& >(*h._ro);
			}
			
			template<typename Arg>
			static constexpr auto& reduce_selfTypes(Arg*, Arg& a){
				static_assert(!std::is_same<Arg,SelfType>::value,"Error: overload resolution failed!");
				return a;
			}
			
			operation_impl(DataStore &ds):ds(ds){}
			
			void act(mtl::TransactionContext* _ctx,typename convert_SelfType<Handle&>::template act<Args>... a){
				auto *ctx = dynamic_cast<typename DataStore::StoreContext*>(_ctx->template get_store_context<DataStore::level>(ds,"operation!").get());
				ds.operation(_ctx,*ctx,OperationIdentifier<Name>{nullptr},
							 this->template reduce_selfTypes(((Args*)nullptr), a)...);
			}
		};

		using operation = std::shared_ptr<operation_super>;
		
		operation op;

		SupportsOn(operation op):op(op){}

		//constructor for use with null handles
		SupportsOn(){}
		
		template<template<typename> class RemoteObject, typename DataStore>
		static operation wrap_operation(DataStore& ds){
			return operation{
				new operation_impl<DataStore,RemoteObject>(ds)};
		}

		virtual Handle& downCast() = 0;
		SupportsOn& upCast(OperationIdentifier<Name>){
			return *this;
		}

		virtual ~SupportsOn(){}
	};
};

}
