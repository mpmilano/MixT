#include <iostream>
#include <chrono>
#include <AtScopeEnd.hpp>
#include <algorithm>
#include <results_header.hpp>
#include <set>
#include <fstream>

auto USE_STRONG(){
	return std::vector<std::vector<struct myria_log> >{{
			runs_USE_STRONG_1(),
				runs_USE_STRONG_2(),
				runs_USE_STRONG_3(), runs_USE_STRONG_4(), runs_USE_STRONG_5(), runs_USE_STRONG_6(), runs_USE_STRONG_7(), runs_USE_STRONG_8(), runs_USE_STRONG_9(), runs_USE_STRONG_10()}};
}

auto NO_USE_STRONG(){
	return std::vector<std::vector<struct myria_log> >{{
			runs_NO_USE_STRONG_1(),
				runs_NO_USE_STRONG_2(),
				runs_NO_USE_STRONG_3(), runs_NO_USE_STRONG_4(), runs_NO_USE_STRONG_5(), runs_NO_USE_STRONG_6(), runs_NO_USE_STRONG_7(), runs_NO_USE_STRONG_8(), runs_NO_USE_STRONG_9(), runs_NO_USE_STRONG_10()}};
}


auto globals_USE_STRONG(){
	return std::vector<std::vector<struct myria_globals> >{{
			globals_USE_STRONG_1(),
				globals_USE_STRONG_2(),
				globals_USE_STRONG_3(), globals_USE_STRONG_4(), globals_USE_STRONG_5(), globals_USE_STRONG_6(), globals_USE_STRONG_7(), globals_USE_STRONG_8(), globals_USE_STRONG_9(), globals_USE_STRONG_10()}};
}

auto globals_NO_USE_STRONG(){
	return std::vector<std::vector<struct myria_globals> >{{
			globals_NO_USE_STRONG_1(),
				globals_NO_USE_STRONG_2(),
				globals_NO_USE_STRONG_3(), globals_NO_USE_STRONG_4(), globals_NO_USE_STRONG_5(), globals_NO_USE_STRONG_6(), globals_NO_USE_STRONG_7(), globals_NO_USE_STRONG_8(), globals_NO_USE_STRONG_9(), globals_NO_USE_STRONG_10()}};
}


using namespace std::chrono;
using namespace mutils;
using std::cout;
using std::endl;
using std::vector;
using std::set;
using namespace std;

double calculate_latency(const std::vector<struct myria_log> &results){
	int total_latency = 0;
	double total_events = 0;
	for (auto &run : results){
		total_events+=1;
		total_latency += run.done_time - run.submit_time;
	}
	
	return total_latency / total_events;
}

template<typename F>
auto window_averages(const std::vector<myria_log> &_results, const F& print_fun){
	constexpr int window_size = duration_cast<milliseconds>(1s).count();
	std::vector<myria_log const *> sorted_results;
	for (auto& r : _results){
		sorted_results.push_back(&r);
	}
	
	std::sort(sorted_results.begin(), sorted_results.end(),
			  [](myria_log const * const a, myria_log const * const b){
				  return a->submit_time < b->submit_time;
			  });
	
	auto print_window_averages =
		[&](const auto window_start, const auto window_end, const auto results_start) {
		int total_latency = 0;
		double total_events = 0;
		AtScopeEnd ase([&](){
				print_fun(std::pair<long,long>{total_events,total_latency / total_events});
			});
		
		for (auto pos = results_start;pos < sorted_results.size(); ++pos){
			auto &run = *sorted_results.at(pos);
			if (run.run_time > window_end) return pos;
			if (run.done_time > window_end) continue;
			total_events += 1;
			total_latency += run.done_time - run.submit_time;
		}
		return sorted_results.size();
	};
	for (auto curr_start_pos = /* 0 */ (sorted_results.size() - sorted_results.size());
		 curr_start_pos < sorted_results.size();
		 curr_start_pos +=
			 print_window_averages(sorted_results.at(curr_start_pos)->submit_time,
								   sorted_results.at(curr_start_pos)->submit_time + window_size,
								   curr_start_pos
				 ));
}

auto print_window_averages(std::string filename, const vector<vector<myria_log> > &all_results){
	ofstream mwlatency;
	mwlatency.open(filename);
	mwlatency << "throughput,latency" << std::endl;

	auto print_pair = [&](const auto &pair){mwlatency << pair.first << "," << pair.second << endl;};
	
	std::vector<std::pair<long,long> > throughput_v_latency;
	for (auto& results : all_results) {
		window_averages(results,
						[&](const auto &pair){throughput_v_latency.push_back(pair);}
			);
	}

	std::sort(throughput_v_latency.begin(),throughput_v_latency.end());
	for (auto &pair : throughput_v_latency){
		print_pair(pair);
	}
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

	std::cout << "number of IO events incurred: normal increments" << std::endl;
	for (auto &results : NO_USE_STRONG()){
		long num_events{0};
		long total_io{0};
		for (auto& row : results){
			if (!row.is_write) continue;
			num_events += 1;
			total_io += row.trackertesting_increment;
		}
		std::cout << "total Increments: " << total_io << std::endl;
		std::cout << "num events: " << num_events << std::endl;
	}

	std::cout << "intended request frequencies:" << std::endl;

	for (auto &results : globals_NO_USE_STRONG()){
		struct FreqCompare {
			bool operator()(const Frequency& lhs, const Frequency& rhs){
				return lhs.hertz < rhs.hertz;
			}
		};
		set<Frequency,FreqCompare> freqs;
		for (auto &row : results){
			freqs.insert(row.request_frequency);
		}
		for (auto &freq : freqs)
			std::cout << freq << std::endl;
	}

	cout << "moving window averages: in mwlatency-(?).csv" << endl;
	print_window_averages("mwlatency-normal.csv",NO_USE_STRONG());
	print_window_averages("mwlatency-strong.csv",USE_STRONG());

}
