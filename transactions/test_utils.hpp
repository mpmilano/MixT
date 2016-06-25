#pragma once
#include <cmath>
#include "Hertz.hpp"
#include "mutils.hpp"

namespace mutils{

//I'm guessing miliseconds.  Here's hoping!
	std::chrono::milliseconds getArrivalInterval(Frequency arrival_rate);

	using milliseconds = decltype(getArrivalInterval(5_Hz));

	unsigned int get_zipfian_value(unsigned int max, double alpha);

	template<typename A, typename B>
	auto micros(std::chrono::duration<A,B> time){
		using namespace std::chrono;
		return duration_cast<microseconds>(time).count();
	}

	using time_point = std::decay_t<decltype(std::chrono::high_resolution_clock::now())>;
	extern const time_point launch_clock;
	using duration = decltype(launch_clock - launch_clock);
		
//was as microseconds
	duration elapsed_time();

#ifndef STRONG_REMOTE_IP
#define STRONG_REMOTE_IP "127.0.0.1"
#endif
	
	constexpr unsigned int get_strong_ip(){
		constexpr char const * const strong_remote_ip{STRONG_REMOTE_IP};
		//if string is non-zero in length
		if (strong_remote_ip[0]){
			return mutils::decode_ip(strong_remote_ip);
		}
		else return mutils::decode_ip("127.0.0.1");
	}
}
