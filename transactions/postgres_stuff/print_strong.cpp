#include <iostream>
#include "sync_defs.h"

int main(){
	std::array<long long,4> to_print;
	auto strong = init_strong();
	select_strong_clock(*strong,to_print);
	std::cout << "Strong known time: " << to_print[0] <<"," << to_print[1] << "," << to_print[2] << "," << to_print[3] <<  std::endl;
}
