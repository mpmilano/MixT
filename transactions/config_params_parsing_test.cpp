#undef DNDEBUG
#include "configuration_params.hpp"
#include "test_utils.hpp"

using namespace myria;
using namespace std;
using namespace chrono;
using namespace mutils;

int main(){
	configuration_parameters params;
	params.strong_ip = get_strong_ip();
	params.causal_ip = get_causal_ip();
	params.strong_relay_port = 8876;
	params.causal_relay_port = 8877;
	params.client_freq = 5_Hz;
	params.starting_num_clients = 30;
	params.increase_clients_freq = 2_Hz;
	params.test_duration = duration_cast<seconds>(100min);
	params.percent_dedicated_connections = .01;
	params.percent_causal = .95;
	params.percent_read = .95;
	params.log_delay_tolerance = 1min;
	params.output_file = "/tmp/MyriaStore-results";
	std::cout << params << std::endl;
	{
		configuration_parameters params;
		std::cin >> params;
		std::cout << params << std::endl;
	}
	exit(0);
}
