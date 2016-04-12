#pragma once
#include <string>
#include "ObjectBuilder.hpp"

namespace myria{

	enum class LoggedStructs {
		log, globals, MAX
			};
	
	enum class LogFields{
		submit_time, run_time, cc_num_tries, done_time, is_write, is_serialization_error, pqxx_failure_string, pqxx_failure, num_causal_tries,
			trackertestingobject_get, trackertestingobject_put, trackertestingobject_isvalid, trackertestingobject_tobytes, trackertestingobject_frombytes,
			trackertesting_exists, trackertesting_constructed, trackertesting_transaction_built, trackertesting_trycast,
			trackertesting_transaction_commit, trackertesting_transaction_abort, trackertesting_localtime, trackertesting_intransaction_check,
			trackertestingobject_constructed, trackertestingobject_registered, trackertesting_newobject, trackertesting_existingobject, trackertesting_existingraw,
			trackertesting_increment, MAX
		};

	enum class GlobalsFields {
		request_frequency, final_completion_time, MAX
	};

	std::ostream& operator<<(std::ostream&, LoggedStructs);
	std::ostream& operator<<(std::ostream&, LogFields);
	std::ostream& operator<<(std::ostream&, GlobalsFields);

	using VMObjectLogger =
		mutils::ObjectBuilder<
		LoggedStructs,
		mutils::StructType<LoggedStructs,LoggedStructs::log,LogFields>,
		mutils::StructType<LoggedStructs,LoggedStructs::globals,GlobalsFields> >;
	std::unique_ptr<VMObjectLogger> build_VMObjectLogger();

	enum class Level { causal, strong, undef};

	constexpr bool runs_with_strong(Level l){
		return (l == Level::strong) || (l == Level::undef);
	}

	constexpr bool runs_with_causal(Level l){
		return l == Level::causal;
	}

	constexpr Level _min_of_levels(){
		return Level::undef;
	}

	template<typename... Levels>
	constexpr Level _min_of_levels(Level l, Levels... a){
		if (l == Level::causal) return l;
		else return _min_of_levels(a...);
	}

	constexpr Level _max_of_levels(){
		return Level::undef;
	}

	template<typename... Levels>
	constexpr Level _max_of_levels(Level l, Levels... a){
		if (l == Level::strong) return l;
		else return _max_of_levels(a...);
	}


	template<typename... Levels>
	constexpr Level min_of_levels(Levels... l){
		Level l1 = _min_of_levels(l...);
		Level l2 = _max_of_levels(l...);
		if (l1 == Level::undef) return l2;
		else return l1;
	}

	template<typename... Levels>
	constexpr Level max_of_levels(Levels... l){
		Level l1 = _min_of_levels(l...);
		Level l2 = _max_of_levels(l...);
		if (l2 == Level::undef) return l1;
		else return l2;
	}

	template<Level l>
	using level_constant = std::integral_constant<Level,l>;


	template<Level l>
	std::string levelStr(){
		const static std::string ret =
			(l == Level::strong ? "strong" :
			 (l == Level::causal ? "weak" : "undef"));
		return ret;
	}

	enum class HandleAccess {read, write, all};

	template<HandleAccess ha>
	using canWrite = std::integral_constant<bool,ha == HandleAccess::write ? true 
											: (ha == HandleAccess::all ? 
											   true : false)>;

	template<HandleAccess ha>
	using canRead = std::integral_constant<bool,
										   ha == HandleAccess::read ? true
										   : (ha == HandleAccess::all ? 
											  true : false)>;

	constexpr bool can_flow(Level from, Level to){
		return to == Level::causal
			|| to == Level::undef
			|| from == Level::strong
			|| from  == Level::undef;
	}

	using Name = long int;

	template<Level l>
	using choose_strong = typename std::integral_constant<bool, runs_with_strong(l)>::type*;

	template<Level l>
	using choose_causal = typename std::integral_constant<bool, runs_with_causal(l)>::type*;

}
