#include <iostream>
#include <results_header.hpp>

auto USE_STRONG(){
	return std::vector<std::vector<log> >{{
			runs_USE_STRONG_1(),
				runs_USE_STRONG_2(),
				runs_USE_STRONG_3(), runs_USE_STRONG_4(), runs_USE_STRONG_5(), runs_USE_STRONG_6(), runs_USE_STRONG_7(), runs_USE_STRONG_8(), runs_USE_STRONG_9(), runs_USE_STRONG_10()}};
}

int main(){
	for (auto &results : USE_STRONG()){
		int total_latency = 0;
		double total_events = 0;
		for (auto &run : results){
			total_events+=1;
			total_latency += run.done_time - run.submit_time;
		}
		std::cout << total_latency / total_events << std::endl;
	}

}
