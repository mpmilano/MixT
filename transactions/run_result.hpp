#pragma once

namespace myria{

	struct run_result{
		using time_t = DECT(std::chrono::high_resolution_clock::now());
		time_t start_time;
		time_t stop_time;
		bool is_write;
		pgsql::Level l;
		bool is_abort{false};
		bool is_protocol_error{false};
		bool is_fatal_error{false};
		void print(const time_t &test_start, std::ostream& o) const {
			using namespace std;
			using namespace chrono;
			o << duration_cast<microseconds>(start_time - test_start).count() << ", "
				<< duration_cast<microseconds>(stop_time - test_start).count() << ", "
				<< is_write << ", " << l << ", " << is_abort << endl;
		}
	};

}
