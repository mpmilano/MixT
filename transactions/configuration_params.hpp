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



	std::ostream& operator<<(std::ostream& o, const std::chrono::seconds& p){
		return o << p.count() << "s";
	}
	
	std::ostream& operator<<(std::ostream& o, const configuration_parameters& p){
		using namespace mutils;
		constexpr mutils::comma_space cs{};
		return o << string_of_ip(p.strong_ip) << cs << p.strong_relay_port << cs <<
			string_of_ip(p.causal_ip) << cs << p.causal_relay_port << cs <<
			p.client_freq << cs << p.starting_num_clients << cs << p.increase_clients_freq << cs <<
			p.test_duration << cs << p.percent_dedicated_connections << cs <<
			p.percent_causal << cs << p.percent_read << cs << p.output_file << cs <<
			p.log_delay_tolerance;
	}

	template<typename U, typename V>
	std::istream& operator>>(std::istream& i, std::chrono::duration<U,V>& p){
		using namespace std;
		using namespace chrono;
		char suffix[] = {0,0,0,0};
		size_t number;
		i >> number >> suffix[0];
		if (suffix[0] == 's'){
			p = duration_cast<DECT(p)>(seconds{number});
		} else {
			i >> suffix[1];
			if (suffix[0] == 'm' && suffix[1] == 'i'){
				i >> suffix[2];
				assert(string{suffix} == "min");
				p = duration_cast<DECT(p)>(minutes{number});
			}
			if (suffix[0] == 'm' && suffix[1] == 's'){
				p = duration_cast<DECT(p)>(milliseconds{number});
			}
			if (suffix[0] == 'u' && suffix[1] == 's'){
				p = duration_cast<DECT(p)>(microseconds{number});
			}
		}
		return i;
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
			p.log_delay_tolerance;
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
      params.strong_ip = decode_ip(args[0]);
    }
    {
      params.strong_relay_port = atoi(args[1]);
    }
    {
      params.causal_ip = decode_ip(args[2]);
    }
    {
      params.causal_relay_port = atoi(args[3]);
    }
    {
      std::istringstream ss{std::string{args[4]}};
      ss >> params.client_freq;
    }
    {
      std::istringstream ss{std::string{args[5]}};
      ss >> params.starting_num_clients;
    }
    {
      std::istringstream ss{std::string{args[6]}};
      ss >> params.increase_clients_freq;
    }
    {
      std::istringstream ss{std::string{args[7]}};
      ss >> params.test_duration;
    }
    {
      std::istringstream ss{std::string{args[8]}};
      ss >> params.percent_dedicated_connections;
    }
    {
      std::istringstream ss{std::string{args[9]}};
      ss >> params.percent_causal;
    }
    {
      std::istringstream ss{std::string{args[10]}};
      ss >> params.percent_read;
    }
    {
			params.output_file = args[11];
    }
		{
      std::istringstream ss{std::string{args[12]}};
      ss >> params.log_delay_tolerance;
    }
	}
}
