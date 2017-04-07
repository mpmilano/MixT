#pragma once
#include "Hertz.hpp"

namespace myria{

struct configuration_parameters{
	//required values
	int strong_ip; int strong_relay_port; int causal_ip; int causal_relay_port;
	mutils::Frequency client_freq; std::size_t starting_num_clients; mutils::Frequency increase_clients_freq;
	std::chrono::seconds test_duration; double percent_dedicated_connections;
	double percent_causal; double percent_read; 
	
	//derived values
	std::size_t max_clients() const {
		return starting_num_clients + (increase_clients_freq.operator*(test_duration));
	}

	template<typename T, typename U>
	std::size_t total_clients_at(std::chrono::duration<T,U> s) const {
		return starting_num_clients + (increase_clients_freq.operator*(s));
	}

	template<typename T, typename U>
	mutils::Frequency current_arrival_rate(std::chrono::duration<T,U> s){
		return client_freq.operator*(total_clients_at(s));
	}
	
	template<typename T, typename U>
	std::size_t clients_still_inactive_at(std::chrono::duration<T,U> s) const {
		return max_clients() - total_clients_at(s);
	}

	std::size_t num_dedicated_connections() const {
		return max_clients() * percent_dedicated_connections;
	}
	
	std::size_t num_spare_connections() const {
		return max_clients() - num_dedicated_connections();
	}
	
};
}
