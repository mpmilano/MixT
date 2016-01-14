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
		private:
			std::shared_ptr<std::mutex> m{std::make_shared<std::mutex>()};
			std::shared_ptr<cache_t> cache{ std::make_shared<cache_t>()};
			std::list<Tracker::Nonce> order;
			static constexpr int max_size = 43;
			static constexpr int tp_size = (int{MAX_THREADS} > 0 ? int{MAX_THREADS} : 50);
			#ifdef MAKE_CACHE_REQUESTS
			ctpl::thread_pool tp{tp_size};
			#endif
			void track_with_eviction(Tracker::Nonce, const obj_bundle &);
		public:
			struct NetException : public mutils::StaticMyriaException<MACRO_GET_STR("Error! Cooperative Cache Net Exception!")>{};
			struct ProtocolException : public mutils::StaticMyriaException<MACRO_GET_STR("Error! Cooperative Cache Protocol Exception!")>{};
			CooperativeCache();
			CooperativeCache(const CooperativeCache&) = delete;
			void insert(Tracker::Nonce, const std::map<Name,std::pair<Tracker::Clock, std::vector<char> > > &map);
			void listen_on(int port);
			std::future<obj_bundle> get(const Tracker::Tombstone&, int port);

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
