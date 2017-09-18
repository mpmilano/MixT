#pragma once
#include "mutils/type_utils.hpp"
#include "pgsql/SQLLevels.hpp"

namespace myria{

	namespace run_result_details{
		using namespace std;
		using namespace chrono;
	struct run_result{
		using time_t = DECT(std::chrono::high_resolution_clock::now());
		time_t start_time{0s};
		time_t stop_time{0s};
		bool is_write{false};
		pgsql::Level l;
		bool is_abort{false};
		std::string abort_string;
		bool is_protocol_error{false};
		bool is_fatal_error{false};
		std::chrono::microseconds slept_for{0u};
		std::chrono::microseconds desired_delay{0us};
		std::chrono::microseconds effective_delay{0us};
		void print(const time_t &test_start, std::ostream& o) const;
		void read(const time_t &test_start, std::istream& i);
	};

	}

	using run_result_details::run_result;

	std::istream& operator>>(std::istream& i, run_result& p);

}
