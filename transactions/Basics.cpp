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
			LogFields::is_serialization_error,"bool is_serialization_error","false",
			LogFields::pqxx_failure_string,"std::string pqxx_failure_string","\"\"",
			LogFields::pqxx_failure,"bool pqxx_failure","false",
			LogFields::num_causal_tries,"int num_causal_tries", "0",
			LogFields::trackertestingobject_get, "int trackertestingobject_get","0",
			LogFields::trackertestingobject_put, "int trackertestingobject_put","0",
			LogFields::trackertestingobject_isvalid, "int trackertestingobject_isvalid","0",
			LogFields::trackertestingobject_tobytes, "int trackertestingobject_tobytes","0",
			LogFields::trackertestingobject_frombytes, "int trackertestingobject_frombytes","0",
			LogFields::trackertesting_exists, "int trackertesting_exists","0",
			LogFields::trackertesting_constructed, "int trackertesting_constructed","0",
			LogFields::trackertesting_transaction_built, "int trackertesting_transaction_built","0",
			LogFields::trackertesting_trycast, "int trackertesting_trycast","0",
			LogFields::trackertesting_transaction_commit, "int trackertesting_transaction_commit","0",
			LogFields::trackertesting_transaction_abort, "int trackertesting_transaction_abort","0",
			LogFields::trackertesting_localtime, "int trackertesting_localtime","0",
			LogFields::trackertesting_intransaction_check, "int trackertesting_intransaction_check","0",
			LogFields::trackertestingobject_constructed, "int trackertestingobject_constructed","0",
			LogFields::trackertestingobject_registered, "int trackertestingobject_registered","0",
			LogFields::trackertesting_newobject, "int trackertesting_newobject","0",
			LogFields::trackertesting_existingobject, "int trackertesting_existingobject","0",
			LogFields::trackertesting_existingraw, "int trackertesting_existingraw","0",
			LogFields::trackertesting_increment, "int trackertesting_increment","0",

			LogFields::transaction_action, "bool transaction_action", "false",

			LogFields::tracker_strong_afterread_tombstone_exists, "int tracker_strong_afterread_tombstone_exists", "0",
			LogFields::tracker_strong_afterread_nonce_unavailable, "int tracker_strong_afterread_nonce_unavailable", "0",
			LogFields::tracker_causal_afterread_candidate, "int tracker_causal_afterread_candidate", "0",
			
			LoggedStructs::globals,"struct myria_globals","ERROR",
			GlobalsFields::request_frequency,"::mutils::Frequency request_frequency","0",
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
		case LogFields::trackertestingobject_get:
			return os << "int trackertestingobject_get";
		case LogFields::trackertestingobject_put:
			return os << "int trackertestingobject_put";
		case LogFields::trackertestingobject_isvalid:
			return os << "int trackertestingobject_isvalid";
		case LogFields::trackertestingobject_tobytes:
			return os << "int trackertestingobject_tobytes";
		case LogFields::trackertestingobject_frombytes:
			return os << "int trackertestingobject_frombytes";
		case LogFields::trackertesting_exists:
			return os << "int trackertesting_exists";
		case LogFields::trackertesting_constructed:
			return os << "int trackertesting_constructed";
		case LogFields::trackertesting_transaction_built:
			return os << "int trackertesting_transaction_built";
		case LogFields::trackertesting_trycast:
			return os << "int trackertesting_trycast";
		case LogFields::trackertesting_transaction_commit:
			return os << "int trackertesting_transaction_commit";
		case LogFields::trackertesting_transaction_abort:
			return os << "int trackertesting_transaction_abort";
		case LogFields::trackertesting_localtime:
			return os << "int trackertesting_localtime";
		case LogFields::trackertesting_intransaction_check:
			return os << "int trackertesting_intransaction_check";
		case LogFields::trackertestingobject_constructed:
			return os << "int trackertestingobject_constructed";
		case LogFields::trackertestingobject_registered:
			return os << "int trackertestingobject_registered";
		case LogFields::trackertesting_newobject:
			return os << "int trackertesting_newobject";
		case LogFields::trackertesting_existingobject:
			return os << "int trackertesting_existingobject";
		case LogFields::trackertesting_existingraw:
			return os << "int trackertesting_existingraw";
		case LogFields::trackertesting_increment:
			return os << "int trackertesting_increment";
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
