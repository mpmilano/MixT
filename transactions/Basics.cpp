#include "Basics.hpp"

namespace myria{
	std::unique_ptr<VMObjectLogger> build_VMObjectLogger() {
		return std::make_unique<VMObjectLogger>(LoggedStructs::log,"struct log","ERROR",
										   LogFields::submit_time, "int submit_time","0",
										   LogFields::run_time, "int run_time","0",
										   LogFields::cc_num_tries, "int cc_num_tries","0",
										   LogFields::done_time, "int done_time","0",
										   LogFields::is_write, "bool is_write","false",
										   LogFields::is_serialization_error,"bool is_serialization_error","false",
										   LogFields::pqxx_failure_string,"std::string pqxx_failure_string","",
										   LogFields::pqxx_failure,"bool pqxx_failure","false",
										   LogFields::num_causal_tries,"int num_causal_tries", "0");
	}

}
