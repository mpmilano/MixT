#pragma once
#include "Basics.hpp"
#include "RemoteObject.hpp"
#include "Handle.hpp"
#include "mutils/CTString.hpp"

/*
  So you deleted half of this framework in the great MTL purge, which includes useful things like how to use this file.
  The directory drafts/ has an early version of this framework and a demo, which is useful for figuring out what the design was supposed to do.
  Check git history for the file mtl/Operate.hpp if you'd to see how this file was once integrated in (old) MTL. 

  As far as I can recall, the basic idea here is taht the Handles all extend the struct SupportsOn, which provides the functions upCast and downCast. 
  UpCast transforms an arbitrary handle into a SupportsOn, which should allow you to access important information like all of the arguments + return type for this operation.
  Most importantly, SupportsOn contains a field `op`, which encapsulates the actual operation behavior (it's literally proxying the call directly to a registered data store).
  That field `op` contains the function act(), which is the thing that *actually does* the operation in question directly at the underlying data store. 

  To review; if you have an operation Name you want to do on a Handle h, call h.upCast(OperationIdentifier<Name>{}).op->act(args...).
  If you are just curious about what operation h has for Name, DECT(h.upCast(OperationIdentifier<Name>{})) has a bunch of useful typedefs in it.
 */

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

		using Return = Ret;
		using OperationName = Name;
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
		  static_assert(std::is_base_of<GDataStore, DataStore>::value);
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
				auto *ctx = dynamic_cast<typename DataStore::StoreContext*>(&_ctx->store_context(ds whendebug(, typename OperationIdentifier<Name>::name{}.string)));
				return ds.operation(_ctx,*ctx,OperationIdentifier<Name>{},
									this->template reduce_selfTypes(((Args*)nullptr), a)...);
			}

			std::nullptr_t act(std::false_type*, TransactionContext* _ctx,typename convert_SelfType<Handle&>::template act<Args>... a){
			  auto *ctx = dynamic_cast<typename DataStore::StoreContext*>(&_ctx->store_context(ds whendebug(, typename OperationIdentifier<Name>::name{}.string)));
				ds.operation(_ctx,*ctx,OperationIdentifier<Name>{},
									this->template reduce_selfTypes(((Args*)nullptr), a)...);
				return nullptr;
			}

			return_t act(TransactionContext* _ctx,typename convert_SelfType<Handle&>::template act<Args>... a){
				std::integral_constant<bool,std::is_same<return_t,return_raw>::value> *choice{nullptr};
				return act(choice,_ctx,a...);
			}

		};

		using operation = std::shared_ptr<operation_super>;
		
		operation op;

		SupportsOn(operation op):op(op){assert(op);}
		SupportsOn(const SupportsOn&) = default;
		SupportsOn& operator=(const SupportsOn&) = default;
		
		template<template<typename> class RemoteObject, typename DataStore>
		static operation wrap_operation(DataStore& ds){
			return operation{
				new operation_impl<DataStore,RemoteObject>(ds)};
		}

		using op_identifier = OperationIdentifier<Name>;
		virtual Handle& downCast() = 0;
		SupportsOn& upCast(OperationIdentifier<Name>){
			return *this;
		}

		template<typename T>
		static auto ifMatches(){
			if constexpr (std::is_same<T,op_identifier>::value){
					return mutils::identity_struct<SupportsOn>{};
				}
			else {
				static const mutils::mismatch ret;
				return ret;
			}
		}
		

		template<typename T>
		auto& operator||(T&){
			return *this;
		}

		virtual ~SupportsOn(){}
	};
};

	template<typename Handle,typename Name, typename Ret, typename... Args>
	struct handle_supports : std::is_base_of<typename SupportedOperation<Name,Ret,Args...>::template SupportsOn<Handle >, Handle > {};

}
