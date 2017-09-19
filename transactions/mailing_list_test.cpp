#include "mailing_list_example.hpp"
#include "threaded_trial.hpp"
#include "pgsql/SQLStore.hpp"
#include "mtl/transaction.hpp"
#include "mtl/transaction_macros.hpp"
#include "test_utils.hpp"
#include "FinalHeader.hpp"
#include "configuration_params.hpp"

int main(int argc, char **argv) {
	myria::configuration_parameters params;
		assert(argc == 1 || argc == 16);
		if (argc == 1) {
			std::cin >> params;
		} else read_from_args(params,argv + 1);
		std::cout << params << std::endl;
		myria::test<examples::mailing_list_state> t1{params};
		t1.run_test();
		exit(0);
	}
