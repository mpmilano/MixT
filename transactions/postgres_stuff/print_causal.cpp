#include <iostream>
#include "sync_defs.h"

int main(){
	std::array<int,4> to_print;
	auto causal = init_causal();
	select_causal_timestamp(*causal,to_print);
	std::cout << "Causal known time: " << to_print[0] <<"," << to_print[1] << "," << to_print[2] << "," << to_print[3] <<  std::endl;
}
