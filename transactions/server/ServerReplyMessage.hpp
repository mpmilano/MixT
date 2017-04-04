#pragma once
#include "SerializationSupport.hpp"
#include "SerializationMacros.hpp"

namespace myria {namespace server {

		template<typename Name>
		struct CacheEntry : public mutils::ByteRepresentable{
			Name name; std::vector<char> bytes;
			CacheEntry(Name name, std::vector<char> bytes)
								 :name(std::move(name)),bytes(std::move(bytes)){}

			DEFAULT_SERIALIZATION_SUPPORT(CacheEntry,name,bytes);
		};
		
		template<typename Name, typename Store>
		struct ServerReplyMessage  : public mutils::ByteRepresentable {
			whendebug(const std::string name{typeid(ServerReplyMessage).name()});
			whendebug(const std::size_t txn_nonce);
			std::vector<CacheEntry<Name> > cache_updates;
			std::unique_ptr<Store> store;

			ServerReplyMessage(whendebug(std::size_t txn_nonce, ) std::vector<CacheEntry<Name> > cache_updates,std::unique_ptr<Store> store)
				:whendebug(txn_nonce(txn_nonce),) cache_updates(cache_updates),store(std::move(store)){}
			
			static std::unique_ptr<ServerReplyMessage> from_bytes(mutils::DeserializationManager* p, char const * v){
#ifndef NDEBUG
				auto checkname = mutils::from_bytes_noalloc<std::string>(p,v);
				v += mutils::bytes_size(*checkname);
				assert(*checkname == std::string{typeid(ServerReplyMessage).name()});
				auto nonce = *mutils::from_bytes_noalloc<DECT(txn_nonce)>(p,v);
				v += mutils::bytes_size(nonce);
#endif
				auto a2 = mutils::from_bytes_noalloc<std::decay_t<decltype(cache_updates)> >(p,v);
				return std::make_unique<ServerReplyMessage>(whendebug(nonce,)
					std::move(*a2),std::move(mutils::from_bytes<Store>(p,v + mutils::bytes_size(*a2))));
			}

			std::size_t bytes_size() const {
				using namespace mutils;
				return mutils::bytes_size(cache_updates) + store->bytes_size() whendebug(+ mutils::bytes_size(name) + mutils::bytes_size(txn_nonce));
			}

			std::size_t to_bytes(char * v) const {
				auto* orig = v;
				using namespace mutils;
				whendebug(v += mutils::to_bytes(name,v));
				whendebug(v += mutils::to_bytes(txn_nonce,v));
				whendebug(auto* checkpoint = v);
				v += mutils::to_bytes(cache_updates,v);
				assert(v - checkpoint > 0);
				assert((std::size_t)(v - checkpoint) == mutils::bytes_size(cache_updates));
				assert(*mutils::from_bytes<std::string>(nullptr,checkpoint) == type_name<DECT(cache_updates)>());
				v += store->to_bytes(v);
				assert(*mutils::from_bytes<std::string>(nullptr,checkpoint) == type_name<DECT(cache_updates)>());
				assert(v > orig);
				auto ret = (std::size_t) (v - orig);
				assert(ret == bytes_size());
				return ret;
			}

			void post_object(const std::function<void (char const * const,std::size_t)>& f) const {
				using namespace mutils;
				whendebug(mutils::post_object(f,name));
				whendebug(mutils::post_object(f,txn_nonce));
				mutils::post_object(f,cache_updates);
				store->post_object(f);
			}

#ifndef NDEBUG
			void ensure_registered(mutils::DeserializationManager&){}
#endif
		};

		template<typename Store>
		struct ClientRequestMessage : public mutils::ByteRepresentable {
			whendebug(const std::size_t txn_nonce;)
			std::unique_ptr<Store> store;
			//std::context_ptr<typename Store::diff_t> diff;
			ClientRequestMessage(whendebug(decltype(txn_nonce) txn_nonce ,) decltype(store) store)
				:whendebug(txn_nonce(txn_nonce),)store(std::move(store)){}
			ClientRequestMessage(whendebug(decltype(txn_nonce) txn_nonce))
			:whendebug(txn_nonce(txn_nonce), ) store(new Store{}){}
			
			static std::unique_ptr<ClientRequestMessage> from_bytes(mutils::DeserializationManager* p, char const * v){
#ifndef NDEBUG
				auto nonce = *mutils::from_bytes_noalloc<DECT(txn_nonce)>(nullptr,v);
				v += mutils::bytes_size(nonce);
#endif
				return std::make_unique<ClientRequestMessage>(whendebug(nonce ,) mutils::from_bytes<Store>(p,v));
			}

			std::size_t bytes_size() const {
				using namespace mutils;
				return whendebug(mutils::bytes_size(txn_nonce) +) store->bytes_size();
			}
			std::size_t to_bytes(char * v) const {
				using namespace mutils;
#ifndef NDEBUG
				auto s1 = mutils::to_bytes(txn_nonce,v);
				v += s1;
#endif
					return store->to_bytes(v) whendebug(+ s1);
			}
			
			void post_object(const std::function<void (char const * const,std::size_t)>& f) const {
				using namespace mutils;
				whendebug(mutils::post_object(f,txn_nonce));
				return store->post_object(f);
			}

#ifndef NDEBUG
			void ensure_registered(mutils::DeserializationManager&){}
#endif
			
		};
	}}
