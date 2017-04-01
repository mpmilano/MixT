#pragma once
#include "SerializationSupport.hpp"
#include "SerializationMacros.hpp"

namespace myria {namespace server {

		template<typename Name>
		struct CacheEntry : public ByteRepresentable{
			Name name; std::vector<char> bytes;
			DEFAULT_SERIALIZATION_SUPPORT(name,bytes);
		};
		
		template<typename Name, typename Store>
		struct ServerReplyMessage  : public ByteRepresentable {
			std::vector<CacheEntry> cache_updates;
			std::unique_ptr<Store> store;

			ServerReplyMessage(std::vector<CacheEntry> cache_updates,std::unique_ptr<Store> store)
				:cache_updates(cache_updates),store(store){}
			
			static std::unique_ptr<ServerReplyMessage> from_bytes(mutils::DeserializationManager* p, char const * v){
				auto a2 = mutils::from_bytes_noalloc<std::decay_t<decltype(cache_updates)> >(p,v);
				return std::make_unique<ServerReplyMessage>(
					std::move(*a2),std::move(mutils::from_bytes<Store>(p,v + mutils::bytes_size(*a2))));
			}
		};

		template<typename Store>
		struct ClientRequestMessage : public ByteRepresentable {
			std::unique_ptr<Store> store;
			std::context_ptr<typename Store::diff_t> diff;
			
			static std::unique_ptr<ServerReplyMessage> from_bytes(mutils::DeserializationManager* p, char const * v){
				auto a2 = mutils::from_bytes<Store>(p,v);
				return std::make_unique<ServerReplyMessage>(
					std::move(a2),std::move(mutils::from_bytes_noalloc<std::decay_t<decltype(store)> >(p,v + mutils::bytes_size(*a2))));
			}
			
		};
	}}
