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

	struct comma_space {
		constexpr comma_space() = default;
	};

	std::ostream& operator<<(std::ostream& o, const comma_space&){
		return o << ", ";
	}

	std::istream& operator>>(std::istream& o, const comma_space&){
		if (o.peek() == ',')
		{
			char cma;
			o.get(cma);
			assert(cma == ',');
		}
		if (o.peek() == ' ')
		{
			char spc;
			o.get(spc);
			assert(spc == ' ');
		}
		return o;
	}

	std::ostream& operator<<(std::ostream& o, const std::chrono::seconds& p){
		return o << p.count() << "s";
	}
	
	std::ostream& operator<<(std::ostream& o, const configuration_parameters& p){
		using namespace mutils;
		constexpr comma_space cs{};
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
		if (suffix[0] == 'm' && suffix[1] == 'i'){
			i >> suffix[1] >> suffix[2];
			assert(string{suffix} == "min");
			p = duration_cast<DECT(p)>(minutes{number});
		}
		if (suffix[0] == 'm' && suffix[1] == 's'){
			i >> suffix[1];
			p = duration_cast<DECT(p)>(milliseconds{number});
		}
		if (suffix[0] == 'u' && suffix[1] == 's'){
			i >> suffix[1];
			p = duration_cast<DECT(p)>(microseconds{number});
		}
		if (suffix[0] == 's'){
			p = duration_cast<DECT(p)>(seconds{number});
		}

		return i;
	}

	struct comma_is_space : std::ctype<char> {
		//from http://stackoverflow.com/questions/7302996/changing-the-delimiter-for-cin-c
		comma_is_space() : std::ctype<char>(get_table()) {}
		static mask const* get_table()
			{
				static mask rc[table_size];
				rc[(int)','] = std::ctype_base::space;
				rc[(int)'\n'] = std::ctype_base::space;
				return &rc[0];
			}
	};
	
	std::istream& operator>>(std::istream& i, configuration_parameters& p){
		using namespace mutils;
		i.imbue(std::locale(i.getloc(), new comma_is_space()));
		constexpr comma_space cs{};
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
}
