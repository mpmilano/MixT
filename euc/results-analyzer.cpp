#include <iostream>
#include <chrono>
#include <AtScopeEnd.hpp>
#include <algorithm>
#include <results_header.hpp>
#include <unified_results_header.hpp>
#include <set>
#include <list>
#include <fstream>
#include <cassert>
#include <sys/resource.h>

using namespace std::chrono;
using namespace mutils;
using std::cout;
using std::endl;
using std::vector;
using std::set;
using namespace std;


auto& NORMAL(){
	return all_logs_NORMAL();
}

const auto& NO_USE_CAUSAL(){
	//return all_logs_NO_USE_CAUSAL();
	static std::vector<myria_log> ret;
	return ret;
}

const auto& NO_USE_STRONG(){
	//return all_logs_NO_USE_STRONG();
	static std::vector<myria_log> ret;
	return ret;
}

auto window_averages(const std::vector<myria_log> &_results){
    constexpr int window_size = duration_cast<milliseconds>(4s).count();
    constexpr int window_step = duration_cast<milliseconds>(4s).count();

	auto results_sorted_by_complete_time = std::make_unique<std::vector<myria_log const *> >();
    for (auto &result : _results) {
		//skip integer overflows.
		if (result.done_time > 0)
			results_sorted_by_complete_time->push_back(&result);
	}

    std::sort(results_sorted_by_complete_time->begin(),
              results_sorted_by_complete_time->end(),
              [](myria_log const * const left, myria_log const * const right){
        assert(left->done_time > 0);
        assert(right->done_time > 0);
        return left->done_time < right->done_time;
    });
		
    auto max_done_time = results_sorted_by_complete_time->back()->done_time;

    //throughput vs latency
	auto &averages = *(new list<pair<long, long> >{});
    int start_index = 0;
    {
        int window_start = 0;
        for (int window_end = window_step;
             window_end <= max_done_time;
             window_start += window_step, window_end += window_step){
            long total_latency{0};
            long total_events{0};
            for (int i = start_index; i < results_sorted_by_complete_time->size(); ++i){
                auto &result = *(*results_sorted_by_complete_time)[i];
                if (result.done_time < window_start) {
                    ++start_index;
                    continue;
                }
                if (result.done_time > window_end) break;
                if (result.is_serialization_error || result.pqxx_failure) continue;
                total_latency += (result.done_time - result.submit_time);
                ++total_events;
            }
            if (total_events == 0) continue;
            averages.push_back(pair<long,long>(total_events,total_latency));
        }
    }
    return averages;
}

auto print_latencies(std::string filename, const vector<myria_log> &all_results){
	if (all_results.size() == 0) return;
	assert(all_results.size() > 0);
    ofstream latencies;
    latencies.open(filename);

	for (auto result : all_results){
		latencies << result.done_time - result.submit_time << endl;
	}
}

auto print_window_averages(std::string filename, const vector<myria_log> &all_results){
	if (all_results.size() == 0) return;
	assert(all_results.size() > 0);
    ofstream mwlatency;
    mwlatency.open(filename);
    mwlatency << "throughput,latency" << std::endl;

    auto print_pair = [&](const auto &pair){mwlatency << pair.first << "," << pair.second << endl;};

	for (auto& pair : window_averages(all_results)){
		print_pair(pair);
	}
}


int main(){
    const rlim_t kStackSize = 8192L * 1024L * 1024L; //8192mb
    struct rlimit rl;
    int result;

    result = getrlimit(RLIMIT_STACK,&rl);
    assert(result == 0);
    if (rl.rlim_cur < kStackSize){
        rl.rlim_cur = kStackSize;
        result = setrlimit(RLIMIT_STACK,&rl);
        assert(result == 0);
    }
    {
        long transaction_action = 0;
        long total_rows = 0;
        long read_or_write = 0;
		cout << NORMAL().size();
        for (auto &row : NORMAL()){
			total_rows++;
			if (row.transaction_action)
				transaction_action++;
			if (row.is_read || row.is_write)
				read_or_write++;
		}
        std::cout << "Transaction action: " << transaction_action << endl;
        cout << "total: " << total_rows << endl;
        cout << "read or write: " << read_or_write << endl;
    }

    cout << "moving window averages: in mwlatency.csv" << endl;
    print_window_averages("mwlatency.csv",NORMAL());
	cout << "full latencies in latencies.csv" << endl;
	print_latencies("latencies.csv",NORMAL());
    //print_window_averages("mwlatency.csv",NO_USE_CAUSAL());
	//print_window_averages("mwlatency-causal.csv",NO_USE_STRONG());
	cout << "done" << endl;

}
