#pragma once
#include <mutex>
#include <array>
#include <memory>
#include <list>
#include "Tracker.hpp"
#include "TrivialPair.hpp"
#include "ctpl_stl.h"

namespace myria { namespace tracker {
		class CooperativeCache{
		public:
			using obj_bundle = std::vector<Tracker::StampedObject>;
			using lock = std::unique_lock<std::mutex>;
			using cache_t = std::map<Tracker::Nonce, obj_bundle >;
			//private:
			std::shared_ptr<std::mutex> m{std::make_shared<std::mutex>()};
			std::shared_ptr<cache_t> cache{ std::make_shared<cache_t>()};
			std::list<Tracker::Nonce> order;
			static constexpr int max_size = 43;
			const CacheBehaviors active_behavior;
			void track_with_eviction(Tracker::Nonce, const obj_bundle &);
		public:
			struct NetException : public mutils::StaticMyriaException<MACRO_GET_STR("Error! Cooperative Cache Net Exception!")>{};
			struct ProtocolException : public mutils::MyriaException{
				std::string emsg;
				ProtocolException(decltype(emsg) emsg):emsg(emsg){}
				const char* what() const _NOEXCEPT {
					return emsg.c_str();
				}
			};
			struct CacheMiss : public mutils::StaticMyriaException<MACRO_GET_STR("Cooperative Cache Did not contain the desired object")>{};
			CooperativeCache(CacheBehaviors beh);
			CooperativeCache(const CooperativeCache&) = delete;
			void insert(Tracker::Nonce, const std::map<Name,std::pair<Tracker::Clock, std::vector<char> > > &map);
			void listen_on(int port);
			std::future<obj_bundle> get(const Tracker::Tombstone&);
			bool contains(const Tracker::Tombstone&) const;
			bool contains(const Tracker::Nonce&) const;

			//this is not an owning pointer
			static std::vector<char> const * const find(const obj_bundle&,const Name&, const Tracker::Clock &version);
			/* be sure to do these checks:
							//make sure we actually need to find this
							assert(remote_vers->first == name);
							assert(!ends::prec(remote_vers->second, version));
			*/
			
		};
	}
}
