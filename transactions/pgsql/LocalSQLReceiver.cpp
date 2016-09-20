#include "LocalSQLReceiver.hpp"

using namespace myria;
using namespace mutils;
using namespace pgsql;
using namespace local;

int main(){
	SQLReceiver<RCVR_LEVEL> r;
	r.r.loop_until_false_set();
}
