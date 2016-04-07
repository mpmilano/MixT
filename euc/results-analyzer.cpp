#include <iostream>
#include <chrono>
#include <AtScopeEnd.hpp>
#include <algorithm>
#include <results_header.hpp>

auto USE_STRONG(){
	return std::vector<std::vector<struct log> >{{
			runs_USE_STRONG_1(),
				runs_USE_STRONG_2(),
				runs_USE_STRONG_3(), runs_USE_STRONG_4(), runs_USE_STRONG_5(), runs_USE_STRONG_6(), runs_USE_STRONG_7(), runs_USE_STRONG_8(), runs_USE_STRONG_9(), runs_USE_STRONG_10()}};
}

auto NO_USE_STRONG(){
	return std::vector<std::vector<struct log> >{{
			runs_NO_USE_STRONG_1(),
				runs_NO_USE_STRONG_2(),
				runs_NO_USE_STRONG_3(), runs_NO_USE_STRONG_4(), runs_NO_USE_STRONG_5(), runs_NO_USE_STRONG_6(), runs_NO_USE_STRONG_7(), runs_NO_USE_STRONG_8(), runs_NO_USE_STRONG_9(), runs_NO_USE_STRONG_10()}};
}

using namespace std::chrono;
using namespace mutils;
using std::cout;
using std::endl;
using std::vector;

double calculate_latency(const std::vector<struct log> &results){
	int total_latency = 0;
	double total_events = 0;
	for (auto &run : results){
		total_events+=1;
		total_latency += run.done_time - run.submit_time;
	}
	
	return total_latency / total_events;
}

int main(){
	cout << "use only strong: " << endl;
	for (auto &results : USE_STRONG()){
		cout << calculate_latency(results) << " for " << results.size() << " total events" << endl;
	}

	cout << "use both: " << endl;
	for (auto &results : NO_USE_STRONG()){
		cout << calculate_latency(results) << " for " << results.size() << " total events" << endl;
	}

	cout << "moving window averages" << endl;

	const int window_size = duration_cast<milliseconds>(1s).count();
	auto print_window_averages = [&](auto& results) {

		/*std::sort(results.begin(), results.end(),
				  [](const log &a, const log &b){
					  return a.submit_time < b.submit_time;
					  })*/
	
		auto print_window_averages =
		[&](const auto window_start, const auto window_end, const auto results_start) {
			std::cout << "intended request frequency: " << "(not recorded)" << std::endl;
			int total_latency = 0;
			double total_events = 0;
			AtScopeEnd ase([&](){
					std::cout << "Average latency: " << total_latency / total_events << endl;
					cout << "Throughput: " << total_events << endl;
				});
			
			for (auto pos = results_start;pos < results.size(); ++pos){
				auto &run = results.at(pos);
				if (run.run_time > window_end) return pos;
				if (run.done_time > window_end) continue;
				total_events += 1;
				total_latency += run.done_time - run.submit_time;
			}
			return results.size();
		};
		for (auto curr_start_pos = /* 0 */ (results.size() - results.size());
			 curr_start_pos < results.size();
			 curr_start_pos +=
				 print_window_averages(results.at(curr_start_pos).submit_time,
									   results.at(curr_start_pos).submit_time + window_size,
									   curr_start_pos
					 ));
	};

	for (auto& results : NO_USE_STRONG()) print_window_averages(results);
	
}
