#pragma once

#include "Tracker.hpp"
#include <mutex>
#include <array>
#include <memory>
#include <list>
#include "TrivialPair.hpp"


class CooperativeCache{
	std::mutex m;
	using lock = std::unique_lock<std::mutex>;
	using obj_bundle = std::vector<TrivialTriple<int, Tracker::Clock, std::vector<char> > >;
	std::map<Tracker::Nonce, obj_bundle > cache;
	std::list<Tracker::Nonce> order;
	static constexpr int max_size = 43;
	CooperativeCache();
	void respond_to_request(int socket);
	void track_with_eviction(Tracker::Nonce, const obj_bundle &);
public:
	CooperativeCache(const CooperativeCache&) = delete;
	void insert(Tracker::Nonce, const std::map<int,std::pair<Tracker::Clock, std::vector<char> > > &map);
	void listen_on(int port);
	std::unique_ptr<obj_bundle> get(Tracker::Nonce, int port);
};

