#include <algorithm>
#include "read_file.hpp"

using namespace mutils;
using namespace myria;
using namespace std;
using namespace chrono;

int main(int, char** argv){
	auto now = std::chrono::high_resolution_clock::now();
	auto results = read_from_file(now,argv[1]);
	auto compare_start_times = [](const run_result& a, const run_result &b){
		return a.start_time < b.start_time;
	};
	(void)compare_start_times;
	auto compare_stop_times = [](const run_result& a, const run_result &b){
		return a.stop_time < b.stop_time;
	};
	std::sort(std::begin(results), std::end(results), compare_stop_times);
	using duration_t = DECT(results.back().stop_time - results.back().stop_time);
	struct throughput_v_latency{
	  Frequency throughput;
	  duration_t latency;
	  void print(std::ostream& o){
	    o << "[" << throughput << "," << duration_cast<microseconds>(latency) << "]" << std::endl;
	  }
	  throughput_v_latency(DECT(throughput) t, DECT(latency) l)
	    :throughput(t),latency(l){}
	};
	std::vector<throughput_v_latency> bin_averages;
	std::size_t increment_fraction = 10;
	std::size_t bin_size = 1000;
	for (std::size_t i = 0; i < (results.size()/bin_size)/increment_fraction; ++i)
	{
		auto latency = results[((i*increment_fraction)+1)*bin_size].stop_time
			- results[(i*increment_fraction)*bin_size].stop_time;
		bin_averages.emplace_back(
					  Frequency{bin_size * duration_cast<duration_t>(1s).count()
					      /latency.count()},
					  latency);
	}
	for (auto & e : bin_averages) e.print(std::cout);
}
