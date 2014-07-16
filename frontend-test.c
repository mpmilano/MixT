
#include "Backend.h"
#include "Tester.h"
#include <iostream>

typedef backend::DataStore<backend::Level::fastest> myds;

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
		myds tmp;
		std::function<int (std::list<int>)> tmp2 = [](std::list<int> l){return l.front();};
		std::function<int (myds&, foocls&)> fuzz = [](myds &i, foocls& add){add.incr(); return add + i.return_one();};
		foocls tmp3(0);
		std::cout << tmp3 << std::endl;
		tester::registerTestFunction(tmp, tmp2, fuzz, tmp3);
		std::cout << tmp3 << std::endl;
		tmp.newHandle<int>();
		tmp.newHandle(4);
		tmp.newHandle(new foocls(8));
		auto &hfcls = tmp.newHandle(std::unique_ptr<foocls>(new foocls(4)));
		tmp.get(hfcls);
		auto fcls = tmp.take(hfcls);
		tmp.give(hfcls, new foocls(3));
		tmp.incr(hfcls); tmp.incr(hfcls);
		std::cout << "deletes the 3" << std::endl;
		tmp.give(hfcls, std::move(fcls));
		std::cout << "deletes the 4" << std::endl;
		tmp.del(hfcls);
		
		

		std::cout << "destructing whole structure" << std::endl;
	}
	myds tmp;
	int i = tmp.return_one() ;
	std::function<int (std::list<int>)> tmp2 = [](std::list<int> l){return l.front();};
	std::function<int (myds&, int)> fuzz = [](myds &i, int add){return add + i.return_one();};
	tester::registerTestFunction(tmp, tmp2, fuzz, 0);
	return i;
	
}
