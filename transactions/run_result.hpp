#pragma once
#include "mutils/type_utils.hpp"
#include "pgsql/SQLLevels.hpp"

namespace myria{

	struct run_result{
		using time_t = DECT(std::chrono::high_resolution_clock::now());
		time_t start_time;
		time_t stop_time;
		bool is_write;
		pgsql::Level l;
		bool is_abort{false};
		std::string abort_string;
		bool is_protocol_error{false};
		bool is_fatal_error{false};
		std::chrono::microseconds slept_for;
		std::chrono::microseconds desired_delay;
		std::chrono::microseconds effective_delay;
		void print(const time_t &test_start, std::ostream& o) const {
			using namespace std;
			using namespace chrono;
			std::stringstream ss;
			ss << duration_cast<microseconds>(start_time - test_start) << ", "
				<< duration_cast<microseconds>(stop_time - test_start) << ", "
				<< is_write << ", " << l << ", " << is_abort <<", \""  << abort_string << "\", "
				 << is_protocol_error << ", " << is_fatal_error<< ", " << slept_for << ", " << desired_delay << ", "
				 << effective_delay << endl;
			o << ss.str();
		}
		void read(const time_t &test_start, std::istream& i)  {
			using namespace std;
			using namespace chrono;
			microseconds start_offset;
			microseconds stop_offset;
			std::string level;
			std::string pre_abort_string;
			constexpr mutils::comma_space cs{};
			i.imbue(std::locale(i.getloc(), new mutils::comma_is_space()));
			i >> start_offset >> cs >> stop_offset >> cs >>
				is_write >> cs >> level >> cs >> is_abort >> cs >> pre_abort_string >> cs >>
				is_protocol_error >> cs >> is_fatal_error >> cs >> slept_for >> cs >> desired_delay >> cs >> effective_delay >> cs;
			start_time = test_start + start_offset;
			stop_time = test_start + stop_offset;
			l = (level.c_str()[0] == 'c' ? pgsql::Level::causal : pgsql::Level::strong);
			pre_abort_string[pre_abort_string.size()-1] = 0;
			abort_string = pre_abort_string.c_str() + 1;
			assert(start_offset.count() != 0 && stop_offset.count() != 0 && start_offset != stop_offset);
		}
	};

	std::istream& operator>>(std::istream& i, run_result& p){
		p.read(std::chrono::high_resolution_clock::now(),i);
		return i;
	}

}
