#include <iostream>
#include <chrono>
#include <AtScopeEnd.hpp>
#include <algorithm>
#include <results_header.hpp>
#include <set>
#include <list>
#include <fstream>
#include <cassert>
#include <sys/resource.h>

auto USE_STRONG(){
    static const auto rus1 = runs_USE_STRONG_1();
    static const auto rus2 = runs_USE_STRONG_2();
    static const auto rus3 = runs_USE_STRONG_3();
    static const auto rus4 = runs_USE_STRONG_4();
    static const auto rus5 = runs_USE_STRONG_5();
    static const auto rus6 = runs_USE_STRONG_6();
    static const auto rus7 = runs_USE_STRONG_7();
    static const auto rus8 = runs_USE_STRONG_8();
    static const auto rus9 = runs_USE_STRONG_9();
    static const auto rus10 = runs_USE_STRONG_10();

    return std::vector<std::vector<struct myria_log> const * >{{&rus1,&rus2,&rus3,&rus4,&rus5,&rus6,&rus7,&rus8,&rus9,&rus10}};
}

auto NO_USE_STRONG(){

    static const auto rus1 = runs_NO_USE_STRONG_1();
    static const auto rus2 = runs_NO_USE_STRONG_2();
    static const auto rus3 = runs_NO_USE_STRONG_3();
    static const auto rus4 = runs_NO_USE_STRONG_4();
    static const auto rus5 = runs_NO_USE_STRONG_5();
    static const auto rus6 = runs_NO_USE_STRONG_6();
    static const auto rus7 = runs_NO_USE_STRONG_7();
    static const auto rus8 = runs_NO_USE_STRONG_8();
    static const auto rus9 = runs_NO_USE_STRONG_9();
    static const auto rus10 = runs_NO_USE_STRONG_10();

    return std::vector<std::vector<struct myria_log> const * >{{&rus1,&rus2,&rus3,&rus4,&rus5,&rus6,&rus7,&rus8,&rus9,&rus10}};
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
    long total_latency = 0;
    double total_events = 0;
    for (auto &run : results){
        total_events+=1;
        assert(run.done_time > run.submit_time);
        total_latency += run.done_time - run.submit_time;
    }

    return total_latency / total_events;
}

auto window_averages(const std::vector<myria_log> &_results){
    constexpr int window_size = duration_cast<milliseconds>(1s).count();
    constexpr int window_step = duration_cast<milliseconds>(10ms).count();

    std::vector<myria_log const *> results_sorted_by_complete_time;
    for (auto &result : _results) results_sorted_by_complete_time.push_back(&result);
    std::sort(results_sorted_by_complete_time.begin(),
              results_sorted_by_complete_time.end(),
              [](myria_log const * const left, myria_log const * const right){
        assert(left->done_time > 0);
        assert(right->done_time > 0);
        return left->done_time < right->done_time;
    });
    auto max_done_time = results_sorted_by_complete_time.back()->done_time;

    //throughput vs latency
    list<pair<long, long> > averages;
    int start_index = 0;
    {
        int window_start = 0;
        for (int window_end = window_step;
             window_end <= max_done_time;
             window_start += window_step, window_end += window_step){
            long total_latency{0};
            long total_events{0};
            for (int i = start_index; i < results_sorted_by_complete_time.size(); ++i){
                auto &result = *results_sorted_by_complete_time[i];
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

auto print_window_averages(std::string filename, const vector<vector<myria_log> const * > &all_results){
    ofstream mwlatency;
    mwlatency.open(filename);
    mwlatency << "throughput,latency" << std::endl;

    auto print_pair = [&](const auto &pair){mwlatency << pair.first << "," << pair.second << endl;};

	for (auto *result : all_results){
		for (auto& pair : window_averages(*result)){
			print_pair(pair);
		}
	}
}


int main(){
    const rlim_t kStackSize = 2048L * 1024L * 1024L; //2048mb
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
        for (auto &results : NO_USE_STRONG()){

            for (auto &row : *results){
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
        if (results->size() <= 1) continue;
        cout << calculate_latency(*results) << " for " << results->size() << " total events" << endl;
    }

    cout << "use both: " << endl;
    for (auto &results : NO_USE_STRONG()){
        if (results->size() <= 1) continue;
        cout << calculate_latency(*results) << " for " << results->size() << " total events" << endl;
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
