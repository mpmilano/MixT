#pragma once
#include "Hertz.hpp"

namespace mutils{

//I'm guessing miliseconds.  Here's hoping!
auto getArrivalInterval(Frequency arrival_rate) {
	using namespace std;
	using namespace chrono;
	// exponential
	constexpr double thousand = -1000.0;
	double U = better_rand();
	double T = thousand * log(U) / (arrival_rate.hertz / 10);
	unsigned long l = round(T);
	return milliseconds(l);
}

using milliseconds = decltype(getArrivalInterval(5_Hz));

	unsigned int get_zipfian_value(unsigned int max, double alpha){
		double y = better_rand();
		assert (y < 1.1);
		assert (y > -0.1);
		double max_pow = pow(max,1 - alpha);
		double x = pow(max_pow*y,1/(1-alpha));
		return round(x);
	}



template<typename A, typename B>
auto micros(std::chrono::duration<A,B> time){
	using namespace std::chrono;
	return duration_cast<microseconds>(time).count();
}

const auto launch_clock = std::chrono::high_resolution_clock::now();
//was as microseconds
auto elapsed_time() {
	return std::chrono::high_resolution_clock::now() - launch_clock;
};


int get_strong_ip() {
	static int ip_addr{[](){
			std::string static_addr {STRONG_REMOTE_IP};
			if (static_addr.length() == 0) static_addr = "127.0.0.1";
			std::cout << static_addr << std::endl;
			return mutils::decode_ip(static_addr);
		}()};
	return ip_addr;
}
}
