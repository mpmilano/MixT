
#include "Backend.hpp"
#include "Tester.hpp"
#include <iostream>

typedef backend::DataStore myds;

class foocls {
private: 
	int i;
public:
	foocls (int i):i(i){}
	foocls (const foocls &) = delete;
	foocls (const foocls &&fcls):i(fcls.i){}
	operator int () const {return i;}
	void incr (){this->i++;}
	virtual ~foocls() {std::cout << "I'm being deleted! I had a " << i << std::endl; }
};

int main () {
	{
		using namespace backend; 
		myds tmp;
		std::function<int (std::list<int>)> tmp2 = [](std::list<int> l){return l.front();};
		std::function<int (myds&, foocls&)> fuzz = [](myds &, foocls& add){add.incr(); return add + 1;};
		foocls tmp3(0);
		std::cout << tmp3 << std::endl;
		tester::registerTestFunction(tmp, tmp2, fuzz, tmp3);
		std::cout << tmp3 << std::endl;
		tmp.newHandle<Level::fastest, int>();
		tmp.newHandle<Level::fastest>(4);
		tmp.newHandle<Level::fastest>(new foocls(8));
		auto hfcls = tmp.newHandle<Level::strong>(std::unique_ptr<foocls>(new foocls(4)));
		auto strongview = tmp.newConsistency<Level::strong>(hfcls);
		myds::TypedHandle<foocls> &noview = strongview;
		assert(&noview);
		tmp.get(hfcls);
		auto fcls = tmp.take(hfcls);
		tmp.give(hfcls, new foocls(3));
		tmp.incr(hfcls); 
		tmp.incr(hfcls);
		std::cout << "deletes the 3" << std::endl;
		tmp.give(hfcls, std::move(fcls));
		std::cout << "deletes the 4" << std::endl;
		tmp.newConsistency<Level::causal>(hfcls);
		tmp.del(hfcls);
		

		std::cout << "destructing whole structure" << std::endl;
	}
	myds tmp;
	std::function<int (std::list<int>)> tmp2 = [](std::list<int> l){return l.front();};
	std::function<int (myds&, int)> fuzz = [](myds &, int add){return add + 1;};
	tester::registerTestFunction(tmp, tmp2, fuzz, 0);
	return 0;
	
}
