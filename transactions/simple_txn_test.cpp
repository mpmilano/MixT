#include "SQLStore.hpp"
#include "transaction.hpp"
#include "transaction_macros.hpp"
#include "test_utils.hpp"
#include "threaded_trial.hpp"
#include "FinalHeader.hpp"

using namespace myria;
using namespace server;
using namespace pgsql;
using namespace mtl;
using namespace std;
using namespace chrono;
using namespace mutils;

namespace myria{

template<Level l>
void client::txn_read(){
	auto &store = get_store<l>();
	auto hndl = store.template existingObject<int>(get_name_read(0.5));
	using Hndl = DECT(hndl);
	constexpr auto read_trans = TRANSACTION(150 + Hndl::label::int_id::value,let remote x = hndl in {})::WITH(hndl);
	return read_trans.run_optimistic(&dsm,*get_relay<l>().lock(),hndl);
	//return read_trans.run_local(hndl);
}

template<Level l>
void client::txn_write(){
	auto &store = get_store<l>();
	auto hndl = store.template existingObject<int>(get_name_write());
	using Hndl = DECT(hndl);
	constexpr auto incr_trans = TRANSACTION(Hndl::label::int_id::value,let remote x = hndl in {x = x + 1})::WITH(hndl);
	return incr_trans.run_optimistic(&dsm,*get_relay<l>().lock(),hndl);
	//return incr_trans.run_local(hndl);
}
	
template<typename U, typename V>
run_result client::client_action(std::chrono::time_point<U,V> /*action start*/){
	auto& params = t.params;
	Level l = (mutils::better_rand() > params.percent_causal ? Level::strong : Level::causal);
	bool write = mutils::better_rand() > params.percent_read;
	switch(l){
	case Level::strong:
		if (write) txn_write<Level::strong>();
		else txn_read<Level::strong>();
		break;
	case Level::causal:
		if (write) txn_write<Level::causal>();
		else txn_read<Level::causal>();
		break;
	default:
		assert(false);
	};
	return run_result{};
}
}

int main(){
	configuration_parameters params;
	params.strong_ip = get_strong_ip();
	params.causal_ip = get_causal_ip();
	params.strong_relay_port = 8876;
	params.causal_relay_port = 8877;
	params.client_freq = 5_Hz;
	params.starting_num_clients = 30;
	params.increase_clients_freq = 2_Hz;
	params.test_duration = duration_cast<seconds>(2min);
	params.percent_dedicated_connections = .01;
	params.percent_causal = .95;
	params.percent_read = .95;
	test t1{params};
	t1.run_test();
}

