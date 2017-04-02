#pragma once
#include "SerializationSupport.hpp"
#include "SerializationMacros.hpp"

namespace myria {namespace server {

		template<typename Name>
		struct CacheEntry : public ByteRepresentable{
			Name name; std::vector<char> bytes;
			CacheEntry(Name name, std::vector<char> bytes)
								 :name(std::move(name)),bytes(std::move(bytes)){}

			DEFAULT_SERIALIZATION_SUPPORT(CacheEntry,name,bytes);
		};
		
		template<typename Name, typename Store>
		struct ServerReplyMessage  : public ByteRepresentable {
			std::vector<CacheEntry<Name> > cache_updates;
			std::unique_ptr<Store> store;

			ServerReplyMessage(std::vector<CacheEntry<Name> > cache_updates,std::unique_ptr<Store> store)
				:cache_updates(cache_updates),store(std::move(store)){}
			
			static std::unique_ptr<ServerReplyMessage> from_bytes(mutils::DeserializationManager* p, char const * v){
				auto a2 = mutils::from_bytes_noalloc<std::decay_t<decltype(cache_updates)> >(p,v);
				return std::make_unique<ServerReplyMessage>(
					std::move(*a2),std::move(mutils::from_bytes<Store>(p,v + mutils::bytes_size(*a2))));
			}

			std::size_t bytes_size() const {
				using namespace mutils;
				return mutils::bytes_size(cache_updates) + store->bytes_size();
			}

			std::size_t to_bytes(char * v) const {
				auto* orig = v;
				using namespace mutils;
				v += mutils::to_bytes(cache_updates,v);
				v += store->to_bytes(v);
				return v - orig;
			}

			void post_object(const std::function<void (char const * const,std::size_t)>& f) const {
				using namespace mutils;
				mutils::post_object(f,cache_updates);
				store->post_object(f);
			}

#ifndef NDEBUG
			void ensure_registered(DeserializationManager&){}
#endif
		};

		template<typename Store>
		struct ClientRequestMessage : public ByteRepresentable {
			std::unique_ptr<Store> store;
			//std::context_ptr<typename Store::diff_t> diff;
			ClientRequestMessage(decltype(store) store):store(std::move(store)){}
			ClientRequestMessage():store(new Store{}){}
			
			static std::unique_ptr<ClientRequestMessage> from_bytes(mutils::DeserializationManager* p, char const * v){
				return std::make_unique<ClientRequestMessage>(mutils::from_bytes<Store>(p,v));
			}

			std::size_t bytes_size() const {
				using namespace mutils;
				return store->bytes_size();
			}
			std::size_t to_bytes(char * v) const {
				using namespace mutils;
				return store->to_bytes(v);
			}
			
			void post_object(const std::function<void (char const * const,std::size_t)>& f) const {
				using namespace mutils;
				return store->post_object(f);
			}

#ifndef NDEBUG
			void ensure_registered(DeserializationManager&){}
#endif
			
		};
	}}
