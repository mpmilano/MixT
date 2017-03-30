#pragma once
#include "Basics.hpp"
#include "RemoteObject.hpp"
#include "Handle.hpp"
#include "CTString.hpp"

namespace myria{
  
struct SelfType{
	constexpr SelfType() = default;
};

template<typename Name>
struct OperationIdentifier;

	namespace RegisteredOperations{
		using Increment = mutils::String<'i','n','c','r','e','m','e','n','t'>;
		using Clone = mutils::String<'c','l','o','n','e'>;
		using Insert = mutils::String<'i','n','s','e','r','t'>;
	}

	template<>
	struct OperationIdentifier<RegisteredOperations::Increment >{
		constexpr OperationIdentifier() = default;
		using name = RegisteredOperations::Increment;
	};

	template<>
	struct OperationIdentifier<RegisteredOperations::Clone >{
		constexpr OperationIdentifier() = default;
		using name = RegisteredOperations::Clone;
	};

	template<>
	struct OperationIdentifier<RegisteredOperations::Insert >{
		constexpr OperationIdentifier() = default;
		using name = RegisteredOperations::Insert;
	};

template<typename Handle>
struct convert_SelfType{
	
	template<typename candidate>
	using act = std::conditional_t<
		std::is_same<candidate,SelfType>::value,
		Handle,
		candidate>;
};

	template<typename T> struct void_to_nullptr{
		using type = T;
	};

	template<> struct void_to_nullptr<void>{
		using type = std::nullptr_t;
	};


	template<typename Name, typename Ret, typename... Args>
struct SupportedOperation {

	template<typename Handle>
	struct SupportsOn{

		using Return = Ret;
		using return_raw = typename convert_SelfType<Handle>::template act<Ret>;
		using return_t = typename void_to_nullptr<return_raw>::type;

		static_assert(is_handle<Handle>::value);
		using TransactionContext = mtl::PhaseContext<label_from_handle<Handle> >;
	
		struct operation_super {
			virtual return_t act(TransactionContext* _ctx,typename convert_SelfType<Handle&>::template act<Args>...) = 0;
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
			
			return_raw act(std::true_type*, TransactionContext* _ctx,typename convert_SelfType<Handle&>::template act<Args>... a){
				auto *ctx = dynamic_cast<typename DataStore::StoreContext*>(&_ctx->store_context(ds whendebug(, OperationIdentifier<Name>::name::string)));
				return ds.operation(_ctx,*ctx,OperationIdentifier<Name>{},
									this->template reduce_selfTypes(((Args*)nullptr), a)...);
			}

			std::nullptr_t act(std::false_type*, TransactionContext* _ctx,typename convert_SelfType<Handle&>::template act<Args>... a){
				auto *ctx = dynamic_cast<typename DataStore::StoreContext*>(&_ctx->store_context(ds whendebug(, OperationIdentifier<Name>::name::string)));
				ds.operation(_ctx,*ctx,OperationIdentifier<Name>{},
									this->template reduce_selfTypes(((Args*)nullptr), a)...);
				return nullptr;
			}

			return_t act(TransactionContext* _ctx,typename convert_SelfType<Handle&>::template act<Args>... a){
				std::integral_constant<bool,std::is_same<return_t,return_raw>::value> *choice{nullptr};
				return act(choice,_ctx,a...);
			}

			auto write_set(TransactionContext* _ctx,typename convert_SelfType<Handle&>::template act<Args>... a){
				auto *ctx = dynamic_cast<typename DataStore::StoreContext*>(&_ctx->store_context(ds whendebug(, "retrieve a write set")));
				return ds.write_set(*ctx,OperationIdentifier<Name>{}, a...);
			}

			auto read_set(TransactionContext* _ctx,typename convert_SelfType<Handle&>::template act<Args>... a){
				auto *ctx = dynamic_cast<typename DataStore::StoreContext*>(&_ctx->store_context(ds whendebug(, "retrieve a read set")));
				return ds.read_set(*ctx,OperationIdentifier<Name>{}, a...);
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

	template<typename Handle,typename Name, typename Ret, typename... Args>
	struct handle_supports : std::is_base_of<typename SupportedOperation<Name,Ret,Args...>::template SupportsOn<Handle >, Handle > {};

}
