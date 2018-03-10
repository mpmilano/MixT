#include "mtl/transaction.hpp"
#include "mtl/transaction_macros.hpp"
#include "mtl/split_printer.hpp"
#include <iostream>
using namespace myria;
using namespace mtl;
using namespace tracker;
using namespace mutils;

class test_mix_methods {
	int one{1};
	int two{2};
public:
	mixt_method(test_method) (three) mixt_captures(one,two) (
		return one + two + three
    )
};

int main(){
    test_mix_methods tmm;
    ClientTracker<> trk;
    DeserializationManager<> *dsm{nullptr};
    tmm.test_method(trk,dsm,15);
}
