#include "Basics.hpp"

namespace myria{
	std::unique_ptr<VMObjectLogger> build_VMObjectLogger() {
		return std::make_unique<VMObjectLogger>(
			LoggedStructs::log,"struct myria_log","ERROR",
			LogFields::submit_time, "int submit_time","0",
			LogFields::run_time, "int run_time","0",
			LogFields::cc_num_tries, "int cc_num_tries","0",
			LogFields::done_time, "int done_time","0",
			LogFields::is_write, "bool is_write","false",
			LogFields::is_read, "bool is_read","false",
			LogFields::is_strong, "bool is_strong","false",
			LogFields::is_causal, "bool is_causal","false",
			LogFields::is_serialization_error,"bool is_serialization_error","false",
			LogFields::pqxx_failure_string,"std::string pqxx_failure_string","\"\"",
			LogFields::pqxx_failure,"bool pqxx_failure","false",
			LogFields::num_causal_tries,"int num_causal_tries", "0",

			LogFields::transaction_action, "bool transaction_action", "false",

			LogFields::tracker_strong_afterread_tombstone_exists, "int tracker_strong_afterread_tombstone_exists", "0",
			LogFields::tracker_strong_afterread_nonce_unavailable, "int tracker_strong_afterread_nonce_unavailable", "0",
			LogFields::tracker_causal_afterread_candidate, "int tracker_causal_afterread_candidate", "0",
			
			LoggedStructs::globals,"struct myria_globals","ERROR",
			GlobalsFields::request_frequency,"::mutils::Frequency request_frequency","0_Hz",
			GlobalsFields::request_frequency_step,"::mutils::Frequency request_frequency_step","0_Hz",
			GlobalsFields::request_frequency_final,"::mutils::Frequency request_frequency_final","0_Hz",
			GlobalsFields::final_completion_time,"int final_completion_time","0"
			);
	}

	std::ostream& operator<<(std::ostream& os, LoggedStructs ls){
		switch(ls){
		case LoggedStructs::log:
			return os << "log";
		case LoggedStructs::globals:
			return os << "globals";
		case LoggedStructs::MAX:
			return os << "max";
		default:
			std::cerr << "out of range for LoggedStructs: " << static_cast<int>(ls) << std::endl;
			assert(false);
		};
	}
	
	std::ostream& operator<<(std::ostream& os, LogFields lf){
		switch(lf){
		case LogFields::submit_time:
			return os << "int submit_time";
		case LogFields::run_time:
			return os << "int run_time";
		case LogFields::cc_num_tries:
			return os << "int cc_num_tries";
		case LogFields::done_time:
			return os << "int done_time";
		case LogFields::is_write:
			return os << "bool is_write";
		case LogFields::is_serialization_error:
			return os << "bool is_serialization_error";
		case LogFields::pqxx_failure_string:
			return os << "std::string pqxx_failure_string";
		case LogFields::pqxx_failure:
			return os << "bool pqxx_failure";
		case LogFields::num_causal_tries:
			return os << "int num_causal_tries";
		default:
			std::cerr << "out of range for LogFields: " << static_cast<int>(lf) << std::endl;
			assert(false);
		};
	}
	
	std::ostream& operator<<(std::ostream& os, GlobalsFields gf){
		switch (gf){
		case GlobalsFields::request_frequency:
			return os << "::mutils::Frequency request_frequency";
		case GlobalsFields::final_completion_time:
			return os << "int final_completion_time";
		default:
			std::cerr << "out of range for GlobalsFields: " << static_cast<int>(gf) << std::endl;
			assert(false);
		};
	}
}
