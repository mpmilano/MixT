#include "ProcessPool.hpp"
#include <iostream>
#include <unistd.h>

int main(){

	std::function<int (int)> foo = [](int a){ sleep(10); return a + 5;};
	std::vector<std::function<int (int)> > test_funs{{foo}};
	ProcessPool<int, int> p(test_funs);
	std::vector<std::future<std::unique_ptr<int> > > futures;
	for  (int i = 0; i < 300; ++i){
		std::cout << "launching " << i << std::endl;
		futures.push_back(p.launch(0,i));
	}
	for (auto &f : futures){
		std::cout << *f.get() << std::endl;
	}
}
