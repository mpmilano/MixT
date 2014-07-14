
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
	operator int () const {return i;}
	void incr (){this->i++;}
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
	}
	myds tmp;
	int i = tmp.return_one() ;
	std::function<int (std::list<int>)> tmp2 = [](std::list<int> l){return l.front();};
	std::function<int (myds&, int)> fuzz = [](myds &i, int add){return add + i.return_one();};
	tester::registerTestFunction(tmp, tmp2, fuzz, 0);
	return i;
	
}
