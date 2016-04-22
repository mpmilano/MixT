#include <iostream>
#include <chrono>
#include <AtScopeEnd.hpp>
#include <algorithm>
#include <results_header.hpp>
#include <set>
#include <fstream>
#include <cassert>

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

auto trackertesting_total_io(const myria_log &row){
        return row.trackertestingobject_get + row.trackertestingobject_put+ row.trackertestingobject_isvalid+ row.trackertestingobject_tobytes+ row.trackertestingobject_frombytes+ row.trackertesting_exists+ row.trackertesting_constructed+ row.trackertesting_transaction_built+ row.trackertesting_transaction_commit+ row.trackertesting_transaction_abort+ row.trackertesting_localtime+ row.trackertesting_intransaction_check+ row.trackertestingobject_constructed+ row.trackertestingobject_registered+ row.trackertesting_newobject+ row.trackertesting_increment;
}

auto trackertestingobject_calls(const myria_log &row){
	return row.trackertestingobject_get + row.trackertestingobject_put+ row.trackertestingobject_isvalid+ row.trackertestingobject_tobytes+ row.trackertestingobject_frombytes + row.trackertestingobject_constructed+ row.trackertestingobject_registered; 
		;
}

auto trackertesting_nonobject_io_calls(const myria_log &row){
        return row.trackertesting_exists+ row.trackertesting_transaction_commit+ row.trackertesting_transaction_abort+ row.trackertesting_newobject+ row.trackertesting_increment;
}

auto trackertesting_other_calls(const myria_log &row){
        return row.trackertesting_constructed+ row.trackertesting_transaction_built+ row.trackertesting_localtime+ row.trackertesting_intransaction_check;
}

int main(){
	{
		long transaction_action = 0;
		long total_rows = 0;
		long read_or_write = 0;
		for (auto &results : NO_USE_STRONG()){
			
			for (auto &row : results){
				total_rows++;
				if (row.transaction_action)
					transaction_action++;
				if (row.is_read || row.is_write)
					read_or_write++;
			}
		}
		std::cout << "Transaction action: " << transaction_action << endl;
		cout << "total: " << total_rows << endl;
		cout << "read or write: " << read_or_write << endl;
	}
	
	cout << "use only strong: " << endl;
	for (auto &results : USE_STRONG()){
                if (results.size() <= 1) continue;
		cout << calculate_latency(results) << " for " << results.size() << " total events" << endl;
	}

	cout << "use both: " << endl;
	for (auto &results : NO_USE_STRONG()){
                if (results.size() <= 1) continue;
		cout << calculate_latency(results) << " for " << results.size() << " total events" << endl;
	}

	std::cout << "number of IO events incurred: normal increments" << std::endl;
	for (auto &results : NO_USE_STRONG()){
            if (results.size() <= 1) continue;
		long num_events{0};
		long total_io{0};
		long object_calls{0};
		long nonobject_io{0};
		long other{0};
		long existingObject_calls{0};
		for (auto& row : results){
			if (row.is_write) continue;
			object_calls += trackertestingobject_calls(row);
			nonobject_io += trackertesting_nonobject_io_calls(row);
			other += trackertesting_other_calls(row);
			num_events++;
			total_io += trackertesting_total_io(row);
			existingObject_calls += row.trackertesting_existingobject;
		}
		cout << "TrackerTesting API hits: " << total_io << endl;
		std::cout << "num events: " << num_events << std::endl;
		cout << "events per increment: " << (total_io + 0.0) / num_events << endl;
		cout << "object io per increment: " << object_calls / (num_events + 0.0) << endl;
		cout << "nonobject io per increment: " << nonobject_io / (num_events + 0.0) << endl;
		cout << "other API hits: " << other / (num_events + 0.0) << endl;
		cout << "existingobject " << existingObject_calls/ (num_events + 0.0) << endl;
 	}

	std::cout << "intended request frequencies:" << std::endl;

	for (auto &results : globals_NO_USE_STRONG()){
                assert(0_Hz == 0_Hz);
                set<Frequency> freqs;
		for (auto &row : results){
                    if (row.request_frequency != 0_Hz)
			freqs.insert(row.request_frequency);
		}
		for (auto &freq : freqs)
			std::cout << freq << std::endl;
	}

	cout << "moving window averages: in mwlatency-(?).csv" << endl;
	print_window_averages("mwlatency-normal.csv",NO_USE_STRONG());
	print_window_averages("mwlatency-strong.csv",USE_STRONG());

}
