#include "mailing_list_example.hpp"

#include "threaded_trial.hpp"
#include "pgsql/SQLStore.hpp"
#include "mtl/transaction.hpp"
#include "mtl/transaction_macros.hpp"
#include "test_utils.hpp"
#include "FinalHeader.hpp"
#include "configuration_params.hpp"

using namespace myria;
using namespace server;
using namespace pgsql;
using namespace mtl;
using namespace std;
using namespace chrono;
using namespace mutils;
using namespace examples;

namespace examples{
	struct mailing_list_state{
		static groups all_groups();
		std::vector<group> cached_groups;
		std::vector<user_hndl> my_users;
		group& pick_group();
		user_hndl& pick_user();
	};
}

using namespace examples;

namespace myria {

	template<>
	std::unique_ptr<run_result> & client<mailing_list_state>::client_action(std::unique_ptr<run_result> &result) {
		//four options: post message, join group, create user, download_inbox

		auto choice = mutils::better_rand();
		assert(choice > 0);
		assert(choice < 1);
		if (choice < .25){
			i.pick_group().post_new_message(trk, "This is the only message");
		} else if (choice < .5){
			i.pick_group().add_new_user(trk,i.pick_user());
			
		} else if (choice < .75) {
			create_user(trk,i.all_groups());
		}
		else {
			download_inbox(trk,i.pick_user());
		}
	}
	
	int main(int argc, char **argv) {
		configuration_parameters params;
		assert(argc == 1 || argc == 16);
		if (argc == 1) {
			std::cin >> params;
		} else read_from_args(params,argv + 1);
		std::cout << params << std::endl;
		test t1{params};
		t1.run_test();
		exit(0);
	}
