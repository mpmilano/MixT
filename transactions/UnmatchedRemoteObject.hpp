#pragma once

namespace myria{

	using UnmatchedUseException =
		mutils::StaticMyriaException<'u','n','m','a','t','c','h','e','d',' ','c','a','l','l',' ','a','t','t','e','m','p','t','e','d'>;
	
	template<typename l, typename T, typename... ops> struct UnmatchedRemoteObject : public RemoteObject<l,T> {
		std::vector<char> bytes;
		UnmatchedRemoteObject(char const * const v, std::size_t size)
			:bytes{v, v + size}{}

		const GDataStore& store() const;
    GDataStore& store();
    Name name() const;
    const std::array<int,NUM_CAUSAL_GROUPS>& timestamp() const;
	};

	template<typename op>
	struct default_operation_impl{
		static void operation(op*){}
	};

	template<typename...> struct default_operation_impl_list;

	template<> struct default_operation_impl_list<> {
		static void operation(){}
	};
	
	template<typename op, typename... otherops>
	struct default_operation_impl_list<op,otherops...> : public default_operation_impl<op>,
																				 public default_operation_impl_list<otherops...>{
		using default_operation_impl<op>::operation;
		using default_operation_impl_list<otherops...>::operation;
	};
	
	template<typename l, typename T, typename... ops> struct UnmatchedDataStore : public default_operation_impl_list<ops...>{
		using default_operation_impl_list<ops...>::operation;

		static UnmatchedDataStore& inst(){
			static UnmatchedDataStore ret;
			return ret;
		}
	};


	template<typename l, typename T,typename... ops>
	GDataStore& UnmatchedRemoteObject<l,T,ops...>::store(){
		return UnmatchedDataStore<l,T,ops...>::inst();
	}
	
	template<typename l, typename T,typename... ops>
	const GDataStore& UnmatchedRemoteObject<l,T,ops...>::store() const {
		return UnmatchedDataStore<l,T,ops...>::inst();
	}

	template<typename l, typename T,typename... ops>
	Name UnmatchedRemoteObject<l,T,ops...>::name() const {
		assert(false && "Cannot call unmatched things");
		throw UnmatchedUseException{};
	}

	template<typename l, typename T,typename... ops>
	const std::array<int,NUM_CAUSAL_GROUPS>& UnmatchedRemoteObject<l,T,ops...>::timestamp() const {
		assert(false && "Cannot call unmatched things");
		throw UnmatchedUseException{};
	}
	
}
