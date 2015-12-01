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
		private:
			std::mutex m;
			using lock = std::unique_lock<std::mutex>;
			std::map<Tracker::Nonce, obj_bundle > cache;
			std::list<Tracker::Nonce> order;
			static constexpr int max_size = 43;
			static constexpr int tp_size = (int{MAX_THREADS} > 0 ? int{MAX_THREADS} : 50);
			ctpl::thread_pool tp{tp_size};
			void respond_to_request(int socket);
			void track_with_eviction(Tracker::Nonce, const obj_bundle &);
		public:
			struct NetException{};
			struct ProtocolException{};
			CooperativeCache();
			CooperativeCache(const CooperativeCache&) = delete;
			void insert(Tracker::Nonce, const std::map<int,std::pair<Tracker::Clock, std::vector<char> > > &map);
			void listen_on(int port);
			std::future<obj_bundle> get(const Tracker::Tombstone&, int port);

			//this is not an owning pointer
			static std::vector<char>* find(const obj_bundle&,const Name&);
			/* be sure to do these checks:
							//make sure we actually need to find this
							assert(remote_vers->first == name);
							assert(!ends::prec(remote_vers->second, version));
			*/
			
		};
	}
}
