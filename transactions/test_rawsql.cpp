#include "raw_sqlstore/RawSQLStore.hpp"
#include "pgsql/SQLLevels.hpp"
#include "mtl/mtl.hpp"

using namespace myria;

using namespace mutils;

using pgsql::causal;
using pgsql::strong;
using namespace raw_sql_store;

using namespace mtl;

struct sample_transactor {
	
	template<typename Txn, typename T>
	void operator()(Txn&, std::unique_ptr<T>& t) const {
		std::cout << "I have access to pqxx here with rider " << *t << std::endl;
	}
};
int main(){
	RawSQLStore<Label<causal>, sample_transactor > test{"host=/var/run/postgresql/"};
	auto hndl = test.get_handle(std::make_unique<int>(23));
	sample_transactor st;
	DeserializationManager<> dsm;
	constexpr auto txn = TRANSACTION(hndl.sql_command(st))::WITH(hndl,st);
	tracker::ClientTracker<> trk;
	txn.run_local(trk, &dsm, hndl, st);
}
