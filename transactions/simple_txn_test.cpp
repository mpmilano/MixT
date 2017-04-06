#ifndef NOPOOL
#define NOPOOL
#endif
#include "SQLStore.hpp"
#include "transaction.hpp"
#include "transaction_macros.hpp"
#include "test_utils.hpp"
#include "FinalHeader.hpp"

using namespace myria;
using namespace server;
using namespace pgsql;
using namespace mtl;
using namespace std;
using namespace chrono;
using namespace mutils;
template<Level l> using SQLInstanceManager = typename SQLStore<l>::SQLInstanceManager;

template<typename sqlstore>
void txn_loop(sqlstore &store){
		auto hndl = store.template existingObject<int>(get_name_read(0.5));
		using Hndl = DECT(hndl);
		constexpr auto incr_trans = TRANSACTION(Hndl::label::int_id::value,let remote x = hndl in {x = x + 1})::WITH(hndl);
		constexpr auto read_trans = TRANSACTION(150 + Hndl::label::int_id::value,let remote x = hndl in {})::WITH(hndl);
		incr_trans.run_local(hndl);
		read_trans.run_local(hndl);
}

void txn_thread(){
	SQLInstanceManager<Level::causal> sc{get_hostname(Level::causal)};
	SQLInstanceManager<Level::strong> ss{get_hostname(Level::strong)};
	DeserializationManager dsm{{&ss,&sc}};
	while (true){
		txn_loop(sc.inst());
		txn_loop(ss.inst());
		std::cout << "looped" << std::endl;
	}
}

int main(){
	for (int i = 0; i < 100; ++i){
		std::thread{txn_thread}.detach();
	}
	this_thread::sleep_for(60s);
}
