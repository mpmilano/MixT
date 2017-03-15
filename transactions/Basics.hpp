#pragma once
#include <string>
#include "ObjectBuilder.hpp"

namespace myria{
  
  enum class LoggedStructs {
    log, globals, MAX
      };
  
  enum class LogFields{
    submit_time, run_time, cc_num_tries, done_time, is_write, is_read, is_strong,is_causal,
      remote_failure_string, num_causal_tries,
      transaction_action, 
			tracker_strong_afterread_tombstone_exists,
      tracker_strong_afterread_nonce_unavailable,
      tracker_causal_afterread_candidate,item_name,
      MAX
      };
  
  enum class GlobalsFields {
    request_frequency, request_frequency_step, request_frequency_final, final_completion_time, MAX
	};
  
  std::ostream& operator<<(std::ostream&, LoggedStructs);
  std::ostream& operator<<(std::ostream&, LogFields);
  std::ostream& operator<<(std::ostream&, GlobalsFields);
  
  constexpr int num_processes = 50;
  static_assert(num_processes <= 100,"Error: you are at risk of too many open files");
  
  using VMObjectLogger =
    mutils::ObjectBuilder<true,
			  LoggedStructs,
			  mutils::StructType<LoggedStructs,LoggedStructs::log,LogFields>,
			  mutils::StructType<LoggedStructs,LoggedStructs::globals,GlobalsFields> >;

  //using VMObjectLog = typename VMObjectLogger::StructBuilder<LoggedStructs::log>;
    using VMObjectLog = mutils::abs_StructBuilder;
  
    //using VMProfiler = mutils::Profiler;
  
  std::unique_ptr<VMObjectLogger> build_VMObjectLogger();



}
