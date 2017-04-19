#pragma once
#include "Hertz.hpp"
#include <cassert>
#include <locale>
#include <chrono>
#include "mutils.hpp"
#include "type_utils.hpp"

namespace myria{

struct configuration_parameters{
	//required values
	int strong_ip; int strong_relay_port; int causal_ip; int causal_relay_port;
	mutils::Frequency client_freq; std::size_t starting_num_clients; mutils::Frequency increase_clients_freq;
	std::chrono::seconds test_duration; double percent_dedicated_connections;
	double percent_causal; double percent_read; std::string output_file;
	std::chrono::seconds log_delay_tolerance;
	unsigned short log_every_n;
	
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

	bool same_run(const configuration_parameters& p1, const configuration_parameters& p2){
		return p1.strong_ip == p2.strong_ip && p1.strong_relay_port == p2.strong_relay_port && p1.causal_relay_port == p2.causal_relay_port
			&& p1.client_freq == p2.client_freq && p1.starting_num_clients == p2.starting_num_clients
			&& p1.increase_clients_freq == p2.increase_clients_freq && p1.test_duration == p2.test_duration
			&& p1.percent_dedicated_connections == p2.percent_dedicated_connections && p1.percent_read == p2.percent_read
			&& p1.percent_causal == p2.percent_causal && p1.log_every_n == p2.log_every_n;
	}
	
	std::ostream& operator<<(std::ostream& o, const configuration_parameters& p){
		using namespace mutils;
		constexpr mutils::comma_space cs{};
		return o << string_of_ip(p.strong_ip) << cs << p.strong_relay_port << cs <<
			string_of_ip(p.causal_ip) << cs << p.causal_relay_port << cs <<
			p.client_freq << cs << p.starting_num_clients << cs << p.increase_clients_freq << cs <<
			p.test_duration << cs << p.percent_dedicated_connections << cs <<
			p.percent_causal << cs << p.percent_read << cs << p.output_file << cs <<
			p.log_delay_tolerance << cs << p.log_every_n;
	}
	
	std::istream& operator>>(std::istream& i, configuration_parameters& p){
		using namespace mutils;
		i.imbue(std::locale(i.getloc(), new mutils::comma_is_space()));
		constexpr mutils::comma_space cs{};
		std::string strong_ip;
		std::string causal_ip;
		i >> strong_ip >> cs >> p.strong_relay_port >> cs >>
			causal_ip >> cs >> p.causal_relay_port >> cs >>
			p.client_freq >> cs >> p.starting_num_clients >> cs >> p.increase_clients_freq >> cs >>
			p.test_duration >> cs >> p.percent_dedicated_connections >> cs >>
			p.percent_causal >> cs >> p.percent_read >> cs >> p.output_file >> cs >>
			p.log_delay_tolerance >> cs >> p.log_every_n;
		std::cout << strong_ip << std::endl
							<< causal_ip << std::endl;
		p.strong_ip = decode_ip(strong_ip.c_str());
		p.causal_ip = decode_ip(causal_ip.c_str());
		return i;
	} //*/

	void read_from_args(configuration_parameters& params, char** args){
		using namespace std;
		using namespace mutils;
    {
			std::cout << "strong ip decoding: " << args[0] << std::endl;
      params.strong_ip = decode_ip(args[0]);
    }
    {
			std::cout << "strong relay decoding: " << args[1] << std::endl;
      params.strong_relay_port = atoi(args[1]);
    }
    {
			std::cout << "causal ip decoding: " << args[2] << std::endl;
      params.causal_ip = decode_ip(args[2]);
    }
    {
			std::cout << "causal_relay_port decoding: " << args[3] << std::endl;
      params.causal_relay_port = atoi(args[3]);
    }
    {
      std::istringstream ss{std::string{args[4]}};
			std::cout << "client_freq decoding: " << args[4] << std::endl;
      ss >> params.client_freq;
    }
    {
      std::istringstream ss{std::string{args[5]}};
			std::cout << "starting_num_clients decoding: " << args[5] << std::endl;
      ss >> params.starting_num_clients;
    }
    {
      std::istringstream ss{std::string{args[6]}};
			std::cout << "increase_clients_freq decoding: " << args[6] << std::endl;
      ss >> params.increase_clients_freq;
    }
    {
      std::istringstream ss{std::string{args[7]}};
			std::cout << "test_duration decoding: " << args[7] << std::endl;
      ss >> params.test_duration;
    }
    {
      std::istringstream ss{std::string{args[8]}};
			std::cout << "percent_dedicated_connections decoding: " << args[8] << std::endl;
      ss >> params.percent_dedicated_connections;
    }
    {
      std::istringstream ss{std::string{args[9]}};
			std::cout << "percent_causal decoding: " << args[9] << std::endl;
      ss >> params.percent_causal;
    }
    {
      std::istringstream ss{std::string{args[10]}};
			std::cout << "percent_read decoding: " << args[10] << std::endl;
      ss >> params.percent_read;
    }
    {
			std::cout << "output_file decoding: " << args[11] << std::endl;
			params.output_file = args[11];
    }
		{
      std::istringstream ss{std::string{args[12]}};
			std::cout << "log_delay_tolerance decoding: " << args[12] << std::endl;
      ss >> params.log_delay_tolerance;
    }
		{
      std::istringstream ss{std::string{args[13]}};
			std::cout << "log_every_n decoding: " << args[13] << std::endl;
      ss >> params.log_every_n;
    }
	}
}
