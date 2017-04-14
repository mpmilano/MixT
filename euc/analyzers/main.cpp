#include "read_file.hpp"

int main(int, char** argv){
	auto now = std::chrono::high_resolution_clock::now();
	for (const auto & e : myria::read_from_file(now,argv[1])){
		e.print(now,std::cout);
	}
}
