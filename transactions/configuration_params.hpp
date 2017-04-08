#pragma once
#include "Hertz.hpp"

namespace myria{

struct configuration_parameters{
	//required values
	int strong_ip; int strong_relay_port; int causal_ip; int causal_relay_port;
	mutils::Frequency client_freq; std::size_t starting_num_clients; mutils::Frequency increase_clients_freq;
	std::chrono::seconds test_duration; double percent_dedicated_connections;
	double percent_causal; double percent_read; std::string output_file;
	std::chrono::seconds log_delay_tolerance;
	
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

	struct comma_space {
		constexpr comma_space() = default;
	};

	std::ostream& operator<<(std::ostream& o, const comma_space&){
		return o << ", ";
	}

	std::istream& operator>>(std::istream& o, const comma_space&){
		char cma; char spc;
		o >> cma >> spc;
		assert(cma == ',');
		assert(spc == ' ');
		return o;
	}
	
	std::ostream& operator<<(std::ostream& o, const configuration_parameters& p){
		constexpr comma_space cs{};
		return o << p.strong_ip << cs << p.strong_relay_port << cs <<
			p.causal_ip << cs << p.causal_relay_port << cs <<
			p.client_freq << cs << p.starting_num_clients << cs << p.increase_clients_freq << cs <<
			p.test_duration.count() << cs << p.percent_dedicated_connections << cs <<
			p.percent_causal << cs << p.percent_read << cs << p.output_file << cs <<
			p.log_delay_tolerance.count();
	}/*
	std::istream& operator>>(std::istream& i, configuration_parameters& p){
	constexpr comma_space cs;
	return i >> p.strong_ip >> cs >> p.strong_relay_port >> cs >>
			p.causal_ip >> cs >> p.causal_relay_port >> cs >>
			p.client_freq >> cs >> p.starting_num_clients >> cs >> p.increase_clients_freq >> cs >>
			p.test_duration >> cs >> p.percent_dedicated_connections >> cs >>
			p.percent_causal >> cs >> p.percent_read >> cs >> p.output_file >> cs >>
			p.log_delay_tolerance;
	} //*/
}
