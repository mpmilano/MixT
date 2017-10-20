#include <algorithm>
#include "read_file.hpp"

using namespace mutils;
using namespace myria;
using namespace std;
using namespace chrono;

int main(int argc, char **argv) {
  assert(argc >= 4);
  auto now = std::chrono::high_resolution_clock::now();
  auto resultsp = read_from_files(now, argc - 3, argv + 3);
	auto &results = resultsp.second;
	auto &params = resultsp.first;
  auto compare_start_times = [](const run_result &a, const run_result &b) {
    return a.start_time < b.start_time;
  };
  (void)compare_start_times;
  auto compare_stop_times = [](const run_result &a, const run_result &b) {
    return a.stop_time < b.stop_time;
  };
  std::sort(std::begin(results), std::end(results), compare_stop_times);
  using duration_t = DECT(results.back().stop_time - results.back().stop_time);
  struct throughput_v_latency {
    Frequency throughput;
    milliseconds average_latency;
    milliseconds elapsed_time;
    microseconds average_delay;
    microseconds average_desired_delay;
    microseconds average_effective_delay;
    void print(std::ostream &o) {
      o << throughput << "," << average_latency << "," << elapsed_time << ","
        << average_delay << "," << average_desired_delay << ","
        << average_effective_delay << std::endl;
    }
    throughput_v_latency(DECT(throughput) t, DECT(average_latency) l,
                         DECT(elapsed_time) e, DECT(average_delay) d,
                         DECT(average_desired_delay) dd,
                         DECT(average_effective_delay) ed)
        : throughput(t), average_latency(l), elapsed_time(e), average_delay(d),
          average_desired_delay(dd), average_effective_delay(ed) {}
  };
  std::vector<throughput_v_latency> bin_averages;
  std::size_t increment_fraction = atoi(argv[1]);
  std::istringstream arg3(argv[2]);
  milliseconds segment_duration_goal;
  arg3 >> segment_duration_goal;
  for (std::size_t i = 0; i < results.size();) {
    std::size_t bin_size{0};
    auto segment_start_time = results[i].stop_time;
    auto segment_stop_time = segment_start_time + segment_duration_goal;
    std::size_t adjust_window_by{0};
		assert(&(*(results.begin() + i)) == &results[i]);
    for (auto it = results.begin() + i;
         it != results.end() && it->stop_time < segment_stop_time; ++it) {
      if (it->stop_time <
          (segment_start_time + (segment_duration_goal / increment_fraction)))
        ++adjust_window_by;
      ++bin_size;
    }
		struct at_scope_end{
			std::size_t &i; std::size_t &adjust_window_by;
			~at_scope_end(){
				i += adjust_window_by;
			}
		};
		at_scope_end ase{i,adjust_window_by};
		(void) ase;
		if (bin_size)	--bin_size;
		if (bin_size == 0 && i == results.size()-1) break;
		else if (bin_size == 0) {
			//std::cout << "outlier: ";
			//results[i].print(now,std::cout);
			continue;
		}
		assert(i < results.size());
		if (i + bin_size >= results.size()){
			//std::cout << i << " " << bin_size << " " << results.size() << std::endl;
		}
		assert(i + bin_size < results.size());
		assert((results[bin_size + i].stop_time - results[i].stop_time).count() > 0);
    auto segment_duration =
        results[bin_size + i].stop_time - results[i].stop_time;
		assert(segment_duration < segment_duration_goal);
    DECT(segment_duration) total_time{0};
    assert(total_time.count() == 0);
    std::size_t j = 0;
    auto finish_time = now;
    microseconds total_delay = microseconds{0};
    microseconds total_desired_delay = microseconds{0};
    microseconds total_effective_delay = microseconds{0};
    for (auto it = results.begin() + i; j < bin_size; ++it, ++j) {
      total_time += it->stop_time - it->start_time;
      total_delay += it->slept_for;
      total_effective_delay += it->effective_delay;
      total_desired_delay += it->desired_delay;
      if (it->stop_time > finish_time)
        finish_time = it->stop_time;
    }
    if (bin_size > 0) {
			if (bin_size * params.log_every_n * duration_cast<duration_t>(1s).count() / segment_duration.count() == 0)
				std::cout << duration_cast<microseconds>(segment_duration) << std::endl;
			assert(bin_size * params.log_every_n * duration_cast<duration_t>(1s).count() / segment_duration.count() != 0);
      bin_averages.emplace_back(
          Frequency{bin_size * params.log_every_n * duration_cast<duration_t>(1s).count() /
                    segment_duration.count()},
          milliseconds{duration_cast<milliseconds>(total_time).count() /
                       bin_size},
          duration_cast<milliseconds>(finish_time - now),
          microseconds{total_delay.count() / ((int)bin_size)},
          microseconds{total_desired_delay.count() / ((int)bin_size)},
          microseconds{total_effective_delay.count() / ((int)bin_size)});
    }
  }
  auto sort_tvl = [](const auto &l, const auto &r) {
    return l.average_latency < r.average_latency;
  };
  std::sort(std::begin(bin_averages), std::end(bin_averages), sort_tvl);
  for (auto &e : bin_averages)
    e.print(std::cout);
}
