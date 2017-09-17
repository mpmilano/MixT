#pragma once
#include "pgsql/SQLStore.hpp"
#include "tracker/ClientTracker.hpp"

namespace myria{

template<pgsql::Level l> using SQLInstanceManager = typename pgsql::SQLStore<l>::SQLInstanceManager;
using LockedConnection = typename mutils::ResourcePool<mutils::simple_rpc::connection>::LockedResource;
using WeakConnection = typename mutils::ResourcePool<mutils::simple_rpc::connection>::WeakResource;

struct test;

struct client{
	SQLInstanceManager<pgsql::Level::causal> sc;
	SQLInstanceManager<pgsql::Level::strong> ss;
	mutils::DeserializationManager dsm{{&ss,&sc}};
	WeakConnection strong_relay;
	WeakConnection causal_relay;
	test &t;
  tracker::ClientTracker<Label<top>,Label<pgsql::strong>, Label<pgsql::causal>, Label<bottom> > trk;

	template<typename s, typename c>
	client(test &t, s &spool, c &cpool, WeakConnection strong_relay, WeakConnection causal_relay)
		:sc(cpool),ss(spool),strong_relay(std::move(strong_relay)),causal_relay(std::move(causal_relay)),t(t){}

	std::unique_ptr<run_result> & client_action(std::unique_ptr<run_result>&);

	//getters, by level.  true for strong.
	
	auto& get_relay(std::true_type*){
		return strong_relay;
	}

	auto& get_relay(std::false_type*){
		return causal_relay;
	}
	
	template<pgsql::Level l> auto& get_relay(){
		constexpr pgsql::choose_strong<l> is_strong{nullptr};
		return get_relay(is_strong);
	}

	auto& get_store(std::true_type*){
		return ss.inst();
	}

	auto& get_store(std::false_type*){
		return sc.inst();
	}
	
	template<pgsql::Level l> auto& get_store(){
		constexpr pgsql::choose_strong<l> is_strong{nullptr};
		return get_store(is_strong);
	}
};
}
