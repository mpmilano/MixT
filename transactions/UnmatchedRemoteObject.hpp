#pragma once
#include "myria-utils/utils.hpp"
#include "Operations.hpp"

namespace myria{

	using UnmatchedUseException =
		mutils::StaticMyriaException<'u','n','m','a','t','c','h','e','d',' ','c','a','l','l',' ','a','t','t','e','m','p','t','e','d'>;
	

	template<typename op>
	struct default_operation_impl{

		template<typename Ctx1, typename Ctx2, typename... Args>
		static typename op::Return operation(const Ctx1&, const Ctx2&, mutils::DeserializationManager<>* ,OperationIdentifier<typename op::OperationName>, const Args&...){
			assert(false && "Cannot call unmatched things");
			throw UnmatchedUseException{};
		}
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
	
	template<typename l, typename T, typename... ops> struct UnmatchedDataStore : public default_operation_impl_list<ops...>, public DataStore<l>{
		using default_operation_impl_list<ops...>::operation;

		static UnmatchedDataStore& inst(){
			static UnmatchedDataStore ret;
			return ret;
		}

	  UnmatchedDataStore(){}
		
		using label = l;
		using StoreContext = mtl::StoreContext<l>;

		bool in_transaction() const { return false;}
		whendebug(std::string why_in_transaction() const { return "not in transaction";})

		std::unique_ptr<mtl::StoreContext<l>> begin_transaction(whendebug(const std::string &)){
			assert(false && "Cannot call unmatched things");
			throw UnmatchedUseException{};
		}
		
		template<typename T2>
		struct UnmatchedRemoteObject : public RemoteObject<l,T> {
			static_assert(std::is_same<T2,T>::value);
			std::vector<char> bytes;
			UnmatchedRemoteObject(char const * const v, std::size_t size)
				:bytes{v, v + size}{}
			
			const UnmatchedDataStore& store() const{
				return UnmatchedDataStore<l,T,ops...>::inst();
			}
			UnmatchedDataStore& store(){
				return UnmatchedDataStore<l,T,ops...>::inst();
			}
			Name name() const{
				assert(false && "Cannot call unmatched things");
				throw UnmatchedUseException{};
			}
			
			const std::array<long long,NUM_CAUSAL_GROUPS>& timestamp() const{
				assert(false && "Cannot call unmatched things");
				throw UnmatchedUseException{};
			}
			
			bool isValid(mtl::StoreContext<l>*) const {
				assert(false && "Cannot call unmatched things");
				throw UnmatchedUseException{};
			}
			std::shared_ptr<const T> get(mutils::DeserializationManager<>*, mtl::StoreContext<l>*) {
				assert(false && "Cannot call unmatched things");
				throw UnmatchedUseException{};
			}

			virtual std::shared_ptr<RemoteObject<l,T>> create_new(mtl::StoreContext<l>*, const T&) const {
				assert(false && "Cannot call unmatched things");
				throw UnmatchedUseException{};
			}
			
			void put(mtl::StoreContext<l>*,const T&) {
				assert(false && "Cannot call unmatched things");
				throw UnmatchedUseException{};
			}

			std::size_t to_bytes(char*  c) const {
				memcpy(c,bytes.data(),bytes.size());
				return bytes.size();
			}

			void post_object(const std::function<void (char const * const, std::size_t) > &f) const {
				f(bytes.data(),bytes.size());
			}

			whendebug(template<typename DSM> void ensure_registered(DSM&){})

			std::size_t bytes_size() const {
				return bytes.size();
			}

			template<typename... ctxs>
			std::unique_ptr<UnmatchedRemoteObject> from_bytes(mutils::DeserializationManager<ctxs...>*, char const * const ){
				assert(false && "Cannot deserialize directly; call constructor");
				throw UnmatchedUseException{};
			}

			std::size_t serial_uuid() const { return 0;}

			std::unique_ptr<LabelFreeHandle<T> > wrapInHandle(std::shared_ptr<RemoteObject<l,T> >){
				assert(false && "Should never be attempting to re-wrap an unmatched handle!");
				throw UnmatchedUseException{};
			}
			
		};
	};

	template<typename l, typename T, typename... SupportedOperations>
	std::unique_ptr<Handle<l,T,SupportedOperations...> > make_unmatched(char const * const v, std::size_t size){
		using Handle = ::myria::Handle<l,T,SupportedOperations...>;
		using UnmatchedStore = UnmatchedDataStore<l,T,SupportedOperations...>;
		return std::unique_ptr<Handle>{new Handle{std::make_shared<typename UnmatchedStore::template UnmatchedRemoteObject<T> >(v,size),
					UnmatchedStore::inst() }};
	}
	
}
