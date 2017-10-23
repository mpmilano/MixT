#include "mailing_list_example.hpp"
#include "threaded_trial.hpp"
#include "pgsql/SQLStore.hpp"
#include "mtl/transaction.hpp"
#include "mtl/transaction_macros.hpp"
#include "test_utils.hpp"
#include "configuration_params.hpp"

int main(int argc, char** argv){
	assert(argc >= 5);
	if (argc < 5) throw argc;
	myria::configuration_parameters params;
	params.strong_ip = mutils::decode_ip(argv[1]);
	params.strong_relay_port = atoi(argv[2]);
	params.causal_ip = mutils::decode_ip(argv[3]);
	params.causal_relay_port = atoi(argv[4]);
	{
		using namespace mutils;
		using namespace std::chrono;
		params.client_freq = 1_Hz;
		params.starting_num_clients = 1;
		params.increase_clients_freq = 0_Hz;
		params.test_duration = 30s;
		params.percent_dedicated_connections = 1;
		params.percent_causal = 1;
		params.percent_read = 1;
		params.output_file="/tmp/test_mailing_list_log";
		params.log_delay_tolerance = 20s;
		params.log_every_n = 2;
		params.parallel_factor = 1;
	}
	myria::test<examples::mailing_list_state> t1{params};
	t1.push_client();
	std::unique_ptr<myria::client<examples::mailing_list_state> > client;
	t1.client_queue.wait_dequeue(client);
	client->i.pick_group(*client).add_new_user(*client, client->i.pick_user(*client));
	client->i.pick_group(*client).post_new_message(*client, "This is only a test");
	auto inbox = download_inbox(*client,client->i.pick_user(*client));
	for (auto &msg : inbox){
		std::cout << msg << std::endl;
	}
}
