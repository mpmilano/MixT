#pragma once
#include "mutils-tasks/resource_pool.hpp"
#include "mutils-networking/simple_rpc.hpp"
#include "configuration_params.hpp"

namespace myria{

struct relay_connections_pool{
	mutils::ResourcePool<mutils::simple_rpc::connection> pool;
	struct spawn_pack{
		std::mutex connections_lock;
		mutils::simple_rpc::connections c;
		spawn_pack(int ip, int port, int max)
			:c(ip,port,max){}
		auto* spawn_p(){
			std::unique_lock<std::mutex> l{connections_lock};
			return new mutils::simple_rpc::connection(c.spawn());
		}
	};
	relay_connections_pool(const configuration_parameters &params, int ip, int port, int max)
		:pool{params.num_dedicated_connections(), params.num_spare_connections(),
			[connections = std::make_shared<spawn_pack>(ip,port,max)] () mutable {
			return connections->spawn_p();
		}, false}{}
	auto spawn(){
		return pool.acquire();
	}
	auto weakspawn(){
		return pool.acquire_weak();
	}
};

}
