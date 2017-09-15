#include "test_utils.hpp"
#include "Hertz.hpp"
#include <sstream>
#include <iostream>


using namespace mutils;
using namespace std;
using namespace chrono;

int main(int argc, char** argv){
	if (argc == 2){
		Frequency f;
		std::istringstream is{argv[1]};
		is >> f;
		std::cout << f << std::endl;
		std::cout << duration_cast<microseconds>(getArrivalInterval(f)) << std::endl;
	}
}
