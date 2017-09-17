#include "threaded_trial.hpp"
#include "pgsql/SQLStore.hpp"
#include "mtl/transaction.hpp"
#include "mtl/transaction_macros.hpp"
#include "test_utils.hpp"
#include "FinalHeader.hpp"
#include "configuration_params.hpp"

using test = ::myria::test<mutils::mismatch>;

namespace myria {

	namespace experiment{
		
		using namespace myria;
		using namespace server;
		using namespace pgsql;
		using namespace mtl;
		using namespace std;
		using namespace chrono;
		using namespace mutils;
		
		using client = ::myria::client<mutils::mismatch>;

template <Level l> void txn_read(client &c) {
	using namespace tracker;
  auto &store = c.get_store<l>();
  auto hndl = store.template existingObject<int>(nullptr, get_name_read(0.5));
  constexpr auto read_trans = TRANSACTION(remote x = hndl, {})::WITH(hndl);
	using connections = typename DECT(c.trk)::connection_references;
	auto strong_connection = c.get_relay<Level::strong>().lock();
	auto causal_connection = c.get_relay<Level::causal>().lock();
	return read_trans.run_optimistic (c.trk, &c.dsm,
																		connections{ConnectionReference<Label<strong> >{*strong_connection},
																				ConnectionReference<Label<causal> >{*causal_connection}}, hndl);
  // return read_trans.run_local(hndl);
}

template <Level l> void txn_write(client & c) {
	using namespace tracker;
  auto &store = c.get_store<l>();
  auto hndl = store.template existingObject<int>(nullptr, get_name_write());
  constexpr auto incr_trans =
      TRANSACTION(remote x = hndl, x = x + 1)::WITH(hndl);
	using connections = typename DECT(c.trk)::connection_references;
	auto strong_connection = c.get_relay<Level::strong>().lock();
	auto causal_connection = c.get_relay<Level::causal>().lock();
	return incr_trans.run_optimistic(c.trk, &c.dsm,connections{ConnectionReference<Label<strong> >{*strong_connection},ConnectionReference<Label<causal> >{*causal_connection}} , hndl);
  // return incr_trans.run_local(hndl);
}
	}

	template<>
	std::unique_ptr<run_result> & experiment::client::client_action(std::unique_ptr<run_result> &result) {
		using namespace experiment;
  auto &params = t.params;
  Level l = (mutils::better_rand() > params.percent_causal ? Level::strong
                                                           : Level::causal);
  bool write = mutils::better_rand() > params.percent_read;
	if (result){
		result->l = l;
		result->is_write = write;
	}
  try {
    switch (l) {
    case Level::strong:
      if (write)
        txn_write<Level::strong>(*this);
      else
        txn_read<Level::strong>(*this);
      break;
    case Level::causal:
      if (write)
        txn_write<Level::causal>(*this);
      else
        txn_read<Level::causal>(*this);
      break;
    default:
      assert(false);
    };
    if (result) result->is_abort = false;
  } catch (const SerializationFailure &f) {
		if (result){
			result->is_abort = true;
			result->abort_string = f.what();
		}
  }
  if (result) result->stop_time = high_resolution_clock::now();
  return result;
}
}

int main(int argc, char **argv) {
	myria::configuration_parameters params;
  assert(argc == 1 || argc == 16);
  if (argc == 1) {
    std::cin >> params;
  } else read_from_args(params,argv + 1);
  std::cout << params << std::endl;
  test t1{params};
  t1.run_test();
	exit(0);
}
